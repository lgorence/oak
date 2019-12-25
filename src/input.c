#include "input.h"

#include <wlr/types/wlr_touch.h>

#include <stdlib.h>
#include <wlr/types/wlr_pointer.h>

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
