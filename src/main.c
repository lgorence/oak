#include <wlr/backend.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_gtk_primary_selection.h>
#include <wlr/types/wlr_idle.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_xdg_shell_v6.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "input.h"
#include "output.h"
#include "server.h"
#include "view.h"

void layer_shell_new_surface_notify(struct wl_listener *listener, void *data) {
    struct wlr_layer_surface_v1 *layer_surface = data;
    struct oak_server *server = wl_container_of(listener, server, layer_shell_new_surface);
    // TODO: we're just using the 'first' output to make this work for now.
    struct oak_output *output = wl_container_of(server->outputs.next, output, link);
    layer_surface->output = output->wlr_output;
    layer_surface->current = layer_surface->client_pending;
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
    server.layer_shell = wlr_layer_shell_v1_create(server.wl_display);
    wlr_data_device_manager_create(server.wl_display);

    wl_list_init(&server.views);
    server.new_surface.notify = surface_new_notify;
    wl_signal_add(&server.xdg_shell->events.new_surface, &server.new_surface);

    server.layer_shell_new_surface.notify = layer_shell_new_surface_notify;
    wl_signal_add(&server.layer_shell->events.new_surface, &server.layer_shell_new_surface);

    //system("gnome-terminal -- htop &");
    if (fork() == 0) {
        execl("/bin/sh", "/bin/sh", "-c", "alacritty --command htop", (void*)NULL);
    }

    wl_display_run(server.wl_display);

    wl_display_destroy_clients(server.wl_display);
    wl_display_destroy(server.wl_display);

    return 0;
}
