#pragma once

#include <wlr/types/wlr_xdg_shell_v6.h>

struct oak_view {
    struct wl_list link;

    struct oak_server *server;
    struct wlr_xdg_surface_v6 *surface;

    struct wl_listener map;
    struct wl_listener destroy;

    uint32_t x;
    uint32_t y;
};
