#pragma once

#include <wayland-server-core.h>

#include <wlr/types/wlr_compositor.h>

struct oak_server {
    struct wl_display *wl_display;
    struct wl_event_loop *wl_event_loop;

    struct wl_list outputs;
    struct wl_list inputs;
    struct wl_list views;

    struct wl_listener new_output;
    struct wl_listener new_input;
    struct wl_listener new_surface;
    struct wl_listener layer_shell_new_surface;

    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_compositor *compositor;
    struct wlr_seat *seat;
    struct wlr_output_layout *output_layout;
    struct wlr_cursor *cursor;
    struct wlr_xcursor_manager *cursor_mgr;

    struct wlr_xdg_shell *xdg_shell;
    struct wlr_layer_shell_v1 *layer_shell;
};
