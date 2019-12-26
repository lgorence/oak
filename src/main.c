#include <wlr/backend.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_gtk_primary_selection.h>
#include <wlr/types/wlr_idle.h>
#include <wlr/types/wlr_xdg_shell_v6.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "input.h"
#include "output.h"
#include "server.h"
#include "view.h"

static void surface_map_notify(struct wl_listener *listener, void *data) {
    struct oak_view *view = wl_container_of(listener, view, map);
    struct oak_server *server = view->server;
    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat);

    wlr_seat_keyboard_notify_enter(server->seat, view->surface->surface, keyboard->keycodes, keyboard->num_keycodes,
                                   &keyboard->modifiers);
}

static void surface_destroy_notify(struct wl_listener *listener, void *data) {
    struct oak_view *view = wl_container_of(listener, view, map);

    wl_list_remove(&view->map.link);
    wl_list_remove(&view->destroy.link);
    wl_list_remove(&view->link);

    free(view);
}

static void new_surface_notify(struct wl_listener *listener, void *data) {
    struct wlr_xdg_surface_v6 *surface = data;
    struct oak_server *server = wl_container_of(listener, server, new_surface);
    struct oak_view *view = calloc(1, sizeof(struct oak_view));

    view->server = server;
    view->surface = surface;
    view->x = 0;
    view->y = 0;

    view->map.notify = surface_map_notify;
    wl_signal_add(&surface->events.map, &view->map);
    view->destroy.notify = surface_destroy_notify;
    wl_signal_add(&surface->events.destroy, &view->destroy);

    wlr_xdg_toplevel_v6_set_fullscreen(surface, true);
    wlr_xdg_toplevel_v6_set_size(surface, 720, 1440);
    wlr_xdg_toplevel_v6_set_activated(surface, true);

    wl_list_insert(&server->views, &view->link);
}

int main(void) {
    struct oak_server server;

    server.wl_display = wl_display_create();
    assert(server.wl_display);
    server.wl_event_loop = wl_display_get_event_loop(server.wl_display);
    assert(server.wl_event_loop);

    server.backend = wlr_backend_autocreate(server.wl_display, NULL);
    assert(server.backend);

    server.seat = wlr_seat_create(server.wl_display, "seat0");

    wl_list_init(&server.outputs);
    server.new_output.notify = new_output_notify;
    wl_signal_add(&server.backend->events.new_output, &server.new_output);

    wl_list_init(&server.inputs);
    server.new_input.notify = input_new_notify;
    wl_signal_add(&server.backend->events.new_input, &server.new_input);

    const char *socket = wl_display_add_socket_auto(server.wl_display);
    assert(socket);

    if (!wlr_backend_start(server.backend)) {
        fprintf(stderr, "Failed to start backend.\n");
        wl_display_destroy(server.wl_display);
        return 1;
    }

    printf("Running compositor on display '%s'\n", socket);
    setenv("WAYLAND_DISPLAY", socket, true);

    wl_display_init_shm(server.wl_display);
    wlr_gtk_primary_selection_device_manager_create(server.wl_display);
    wlr_idle_create(server.wl_display);

    server.compositor = wlr_compositor_create(server.wl_display, wlr_backend_get_renderer(server.backend));
    server.xdg_shell = wlr_xdg_shell_v6_create(server.wl_display);
    wlr_data_device_manager_create(server.wl_display);

    wl_list_init(&server.views);
    server.new_surface.notify = new_surface_notify;
    wl_signal_add(&server.xdg_shell->events.new_surface, &server.new_surface);

    system("gnome-terminal -- htop &");

    wl_display_run(server.wl_display);

    wl_display_destroy_clients(server.wl_display);
    wl_display_destroy(server.wl_display);

    return 0;
}
