#pragma once

#include <wayland-server-core.h>

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

struct oak_output {
    struct wlr_output *wlr_output;
    struct oak_server *server;
    struct timespec last_frame;

    struct wl_listener destroy;
    struct wl_listener frame;

    struct wl_list link;
};

struct oak_input_device {
    struct wlr_input_device *wlr_input_device;
    struct oak_server *server;

    struct wl_listener destroy;

    struct wl_list link;
};
