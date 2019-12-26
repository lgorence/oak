#pragma once

#include <wlr/types/wlr_input_device.h>

#include "server.h"

struct oak_input_device {
    struct wlr_input_device *wlr_device;
    struct oak_server *server;

    struct oak_keyboard *keyboard;

    // Common events
    struct wl_listener destroy;

    // Pointer events
    struct wl_listener pointer_button;
    struct wl_listener pointer_motion_absolute;

    // Touch events
    struct wl_listener touch_down;

    struct wl_list link;
};

struct oak_keyboard {
    struct oak_input_device *device;

    // Keyboard events
    struct wl_listener keyboard_key;
};

void input_destroy_notify(struct wl_listener *listener, void *data);

void input_new_notify(struct wl_listener *listener, void *data);

void input_new_keyboard(struct oak_input_device *oak_device, struct wlr_input_device *wlr_device);

void keyboard_key_notify(struct wl_listener *listener, void *data);

void pointer_button_notify(struct wl_listener *listener, void *data);

void pointer_motion_absolute_notify(struct wl_listener *listener, void *data);

void touch_down_notify(struct wl_listener *listener, void *data);
