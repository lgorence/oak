#include "view.h"

#include <stdlib.h>

void surface_map_notify(struct wl_listener *listener, void *data) {
    struct oak_view *view = wl_container_of(listener, view, map);
    struct oak_server *server = view->server;
    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat);

    wlr_seat_keyboard_notify_enter(server->seat, view->surface->surface, keyboard->keycodes, keyboard->num_keycodes,
                                   &keyboard->modifiers);
}

void surface_destroy_notify(struct wl_listener *listener, void *data) {
    struct oak_view *view = wl_container_of(listener, view, destroy);

    wl_list_remove(&view->map.link);
    wl_list_remove(&view->destroy.link);
    wl_list_remove(&view->link);

    free(view);
}

void surface_new_notify(struct wl_listener *listener, void *data) {
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
