#include "input.h"

#include <stdlib.h>

void input_destroy_notify(struct wl_listener *listener, void *data) {
    struct oak_input_device *input = wl_container_of(listener, input, destroy);
    wl_list_remove(&input->link);
    wl_list_remove(&input->destroy.link);
    free(input);
}

void new_input_notify(struct wl_listener *listener, void *data) {
    struct oak_server *server = wl_container_of(listener, server, new_input);
    struct wlr_input_device *wlr_input_device = data;
    printf("New input device: %s\n", wlr_input_device->name);

    switch (wlr_input_device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            printf("Detected as keyboard.\n");
            break;
        case WLR_INPUT_DEVICE_POINTER:
            printf("Detected as pointer.\n");
            break;
        case WLR_INPUT_DEVICE_TOUCH:
            printf("Detected as touch.\n");
            break;
        default:
            break;
    }

    struct oak_input_device *input = calloc(1, sizeof(struct oak_input_device));
    input->server = server;
    input->wlr_input_device = wlr_input_device;

    wl_list_insert(&server->inputs, &input->link);
    input->destroy.notify = input_destroy_notify;
    wl_signal_add(&wlr_input_device->events.destroy, &input->destroy);
}

