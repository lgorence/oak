#pragma once

#include <wlr/types/wlr_input_device.h>

#include "server.h"

struct oak_input_device {
    struct wlr_input_device *wlr_input_device;
    struct oak_server *server;

    struct wl_listener destroy;

    struct wl_listener pointer_button;
    struct wl_listener pointer_motion_absolute;

    struct wl_listener touch_down;

    struct wl_list link;
};

void input_destroy_notify(struct wl_listener *listener, void *data);
void new_input_notify(struct wl_listener *listener, void *data);

void pointer_button_notify(struct wl_listener *listener, void *data);
void pointer_motion_absolute_notify(struct wl_listener *listener, void *data);

void touch_down_notify(struct wl_listener *listener, void *data);
