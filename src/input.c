#include "input.h"

#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_touch.h>

#include <stdlib.h>

void input_new_notify(struct wl_listener *listener, void *data) {
    struct oak_server *server = wl_container_of(listener, server, new_input);
    struct wlr_input_device *wlr_input_device = data;
    printf("Input device created: '%s-%s'\n", wlr_input_device->name, wlr_input_device->output_name);

    struct oak_input_device *input = calloc(1, sizeof(struct oak_input_device));
    input->server = server;
    input->wlr_device = wlr_input_device;

    // TODO: probably should 'properly' implement this.
    uint32_t caps = WL_SEAT_CAPABILITY_KEYBOARD | WL_SEAT_CAPABILITY_POINTER;

    switch (wlr_input_device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            printf("Detected as keyboard.\n");
            input_new_keyboard(input, wlr_input_device);
            break;
        case WLR_INPUT_DEVICE_POINTER:
            printf("Detected as pointer.\n");
            input->pointer_button.notify = pointer_button_notify;
            wl_signal_add(&wlr_input_device->pointer->events.button, &input->pointer_button);
            input->pointer_motion_absolute.notify = pointer_motion_absolute_notify;
            wl_signal_add(&wlr_input_device->pointer->events.motion_absolute, &input->pointer_motion_absolute);
            break;
        case WLR_INPUT_DEVICE_TOUCH:
            printf("Detected as touch.\n");
            input->touch_down.notify = touch_down_notify;
            wl_signal_add(&wlr_input_device->touch->events.down, &input->touch_down);
            break;
        default:
            break;
    }

    wlr_seat_set_capabilities(server->seat, caps);

    wl_list_insert(&server->inputs, &input->link);

    input->destroy.notify = input_destroy_notify;
    wl_signal_add(&wlr_input_device->events.destroy, &input->destroy);
}

void input_destroy_notify(struct wl_listener *listener, void *data) {
    struct oak_input_device *input = wl_container_of(listener, input, destroy);
    struct wlr_input_device *wlr_input_device = input->wlr_device;
    printf("Input device destroyed: '%s-%s'\n", wlr_input_device->name, wlr_input_device->output_name);

    wl_list_remove(&input->link);
    wl_list_remove(&input->destroy.link);

    switch (wlr_input_device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            break;
        case WLR_INPUT_DEVICE_POINTER:
            wl_list_remove(&input->pointer_button.link);
            wl_list_remove(&input->pointer_motion_absolute.link);
            break;
        case WLR_INPUT_DEVICE_TOUCH:
            wl_list_remove(&input->touch_down.link);
            break;
        default:
            break;
    }

    free(input);
}

void input_new_keyboard(struct oak_input_device *oak_device, struct wlr_input_device *wlr_device) {
    struct oak_keyboard *keyboard = calloc(1, sizeof(struct oak_keyboard));
    keyboard->device = oak_device;
    oak_device->keyboard = keyboard;

    struct xkb_rule_names rules = {0};
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap = xkb_map_new_from_names(context, &rules,
                                                       XKB_KEYMAP_COMPILE_NO_FLAGS);
    wlr_keyboard_set_keymap(wlr_device->keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
    wlr_keyboard_set_repeat_info(wlr_device->keyboard, 25, 600);
    keyboard->keyboard_key.notify = keyboard_key_notify;
    wl_signal_add(&wlr_device->keyboard->events.key, &keyboard->keyboard_key);
    wlr_seat_set_keyboard(oak_device->server->seat, wlr_device);
}

void input_destroy_keyboard(struct oak_input_device *oak_device) {
    wl_list_remove(&oak_device->keyboard->keyboard_key.link);

    free(oak_device->keyboard);
}

void keyboard_key_notify(struct wl_listener *listener, void *data) {
    struct oak_keyboard *keyboard = wl_container_of(listener, keyboard, keyboard_key);
    struct oak_input_device *device = keyboard->device;
    struct oak_server *server = device->server;
    struct wlr_event_keyboard_key *event = data;

    wlr_seat_set_keyboard(server->seat, device->wlr_device);
    wlr_seat_keyboard_notify_key(server->seat, event->time_msec, event->keycode, event->state);
}

void pointer_button_notify(struct wl_listener *listener, void *data) {
    struct oak_input_device *input = wl_container_of(listener, input, pointer_button);
    struct wlr_event_pointer_button *event = data;

    printf("Pointer button event: %d %s\n", event->button, event->state ? "DOWN" : "UP");
}

void pointer_motion_absolute_notify(struct wl_listener *listener, void *data) {
    struct oak_input_device *input = wl_container_of(listener, input, pointer_button);
    struct wlr_event_pointer_motion_absolute *event = data;

    printf("Pointer absolute motion event: (%f,%f)\n", event->x, event->y);
}

void touch_down_notify(struct wl_listener *listener, void *data) {
    struct oak_input_device *input = wl_container_of(listener, input, touch_down);
}
