#pragma once

#include <wlr/types/wlr_output.h>

#include "server.h"

struct oak_output {
    struct wlr_output *wlr_output;
    struct oak_server *server;
    struct timespec last_frame;

    struct wl_listener destroy;
    struct wl_listener frame;

    struct wl_list link;
};

void output_destroy_notify(struct wl_listener *listener, void *data);

void output_frame_notify(struct wl_listener *listener, void *data);

void new_output_notify(struct wl_listener *listener, void *data);
