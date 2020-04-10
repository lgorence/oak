#pragma once

#include <wlr/types/wlr_xdg_shell_v6.h>

#include "server.h"

struct oak_view {
    struct wl_list link;

    struct oak_server *server;
    struct wlr_xdg_surface_v6 *surface;

    struct wl_listener map;
    struct wl_listener destroy;

    uint32_t x;
    uint32_t y;
};

void surface_map_notify(struct wl_listener *listener, void *data);

void surface_destroy_notify(struct wl_listener *listener, void *data);

void surface_new_notify(struct wl_listener *listener, void *data);

struct oak_view *surface_get_view_for_coord(struct oak_server *server, uint32_t x, uint32_t y);
