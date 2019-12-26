#include "input.h"

#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_touch.h>

#include <stdlib.h>

void new_input_notify(struct wl_listener *listener, void *data) {
    struct oak_server *server = wl_container_of(listener, server, new_input);
    struct wlr_input_device *wlr_input_device = data;
    printf("Input device created: '%s-%s'\n", wlr_input_device->name, wlr_input_device->output_name);

    struct oak_input_device *input = calloc(1, sizeof(struct oak_input_device));
    input->server = server;
    input->wlr_input_device = wlr_input_device;

    switch (wlr_input_device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            printf("Detected as keyboard.\n");
            struct xkb_rule_names rules = {0};
            struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
            struct xkb_keymap *keymap = xkb_map_new_from_names(context, &rules,
                                                               XKB_KEYMAP_COMPILE_NO_FLAGS);
            wlr_keyboard_set_keymap(wlr_input_device->keyboard, keymap);
            xkb_keymap_unref(keymap);
            xkb_context_unref(context);
            wlr_keyboard_set_repeat_info(wlr_input_device->keyboard, 25, 600);
            input->keyboard_key.notify = keyboard_key_notify;
            wl_signal_add(&wlr_input_device->keyboard->events.key, &input->keyboard_key);
            wlr_seat_set_keyboard(server->seat, wlr_input_device);
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

    wl_list_insert(&server->inputs, &input->link);

    input->destroy.notify = input_destroy_notify;
    wl_signal_add(&wlr_input_device->events.destroy, &input->destroy);
}

void input_destroy_notify(struct wl_listener *listener, void *data) {
    struct oak_input_device *input = wl_container_of(listener, input, destroy);
    struct wlr_input_device *wlr_input_device = input->wlr_input_device;
    printf("Input device destroyed: '%s-%s'\n", wlr_input_device->name, wlr_input_device->output_name);

    wl_list_remove(&input->link);
    wl_list_remove(&input->destroy.link);

    switch (wlr_input_device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            wl_list_remove(&input->keyboard_key.link);
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

void keyboard_key_notify(struct wl_listener *listener, void *data) {
    struct oak_input_device *input = wl_container_of(listener, input, keyboard_key);
    struct oak_server *server = input->server;
    struct wlr_event_keyboard_key *event = data;

    wlr_seat_set_keyboard(server->seat, input->wlr_input_device);
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
