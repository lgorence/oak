#pragma once

#include <wlr/types/wlr_input_device.h>

#include "server.h"

struct oak_input_device {
    struct wlr_input_device *wlr_input_device;
    struct oak_server *server;

    struct wl_listener destroy;

    struct wl_list link;
};

void input_destroy_notify(struct wl_listener *listener, void *data);
void new_input_notify(struct wl_listener *listener, void *data);
