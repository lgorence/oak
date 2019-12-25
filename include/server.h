#pragma once

#include <wayland-server-core.h>

#include <wlr/types/wlr_compositor.h>

struct oak_server {
    struct wl_display *wl_display;
    struct wl_event_loop *wl_event_loop;

    struct wl_listener new_output;
    struct wl_list outputs;

    struct wl_listener new_input;
    struct wl_list inputs;

    struct wlr_backend *backend;
    struct wlr_compositor *compositor;
};
