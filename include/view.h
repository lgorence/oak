#pragma once

#include <wlr/types/wlr_xdg_shell.h>

#include "server.h"

struct oak_view {
    struct wl_list link;

    struct oak_server *server;
    struct wlr_xdg_surface *surface;

    struct wl_listener map;
    struct wl_listener destroy;

    uint32_t x;
    uint32_t y;
};

void surface_map_notify(struct wl_listener *listener, void *data);

void surface_destroy_notify(struct wl_listener *listener, void *data);

void surface_new_notify(struct wl_listener *listener, void *data);
