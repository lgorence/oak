#include "server.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_gtk_primary_selection.h>
#include <wlr/types/wlr_idle.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_xdg_shell_v6.h>

static void output_destroy_notify(struct wl_listener *listener, void *data) {
    struct oak_output *output = wl_container_of(listener, output, destroy);
    wl_list_remove(&output->link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->frame.link);
    free(output);
}

static void output_frame_notify(struct wl_listener *listener, void *data) {
    struct oak_output *output = wl_container_of(listener, output, frame);
    struct oak_server *server = output->server;
    struct wlr_output *wlr_output = data;
    struct wlr_renderer *renderer = wlr_backend_get_renderer(wlr_output->backend);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    wlr_output_attach_render(wlr_output, NULL);
    wlr_renderer_begin(renderer, wlr_output->width, wlr_output->height);

    float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    wlr_renderer_clear(renderer, color);

    struct wl_resource *_surface;
    wl_resource_for_each(_surface, &server->compositor->surface_resources) {
        struct wlr_surface *surface = wlr_surface_from_resource(_surface);
        if (!wlr_surface_has_buffer(surface)) {
            continue;
        }
        struct wlr_box render_box = {
            .x = 20, .y = 20,
            .width = surface->current.width,
            .height = surface->current.height
        };
        float matrix[16];
        struct wlr_texture *texture = wlr_surface_get_texture(surface);
        wlr_matrix_project_box(&matrix, &render_box, surface->current.transform, 0, &wlr_output->transform_matrix);
        wlr_render_texture_with_matrix(renderer, texture, &matrix, 1.0f);
        wlr_surface_send_frame_done(surface, &now);
    }

    wlr_renderer_end(renderer);
    wlr_output_commit(wlr_output);
}

static void new_output_notify(struct wl_listener *listener, void *data) {
    struct oak_server *server = wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_output = data;

    if (!wl_list_empty(&wlr_output->modes)) {
        struct wlr_output_mode *mode = wl_container_of(wlr_output->modes.prev, mode, link);
        wlr_output_set_mode(wlr_output, mode);
    }

    struct oak_output *output = calloc(1, sizeof(struct oak_output));
    clock_gettime(CLOCK_MONOTONIC, &output->last_frame);
    output->server = server;
    output->wlr_output = wlr_output;

    wl_list_insert(&server->outputs, &output->link);
    output->destroy.notify = output_destroy_notify;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);
    output->frame.notify = output_frame_notify;
    wl_signal_add(&wlr_output->events.frame, &output->frame);

    wlr_output_create_global(wlr_output);
}

static void input_destroy_notify(struct wl_listener *listener, void *data) {
    struct oak_input_device *input = wl_container_of(listener, input, destroy);
    wl_list_remove(&input->link);
    wl_list_remove(&input->destroy.link);
    free(input);
}

static void new_input_notify(struct wl_listener *listener, void *data) {
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

int main(void) {
    struct oak_server server;

    server.wl_display = wl_display_create();
    assert(server.wl_display);
    server.wl_event_loop = wl_display_get_event_loop(server.wl_display);
    assert(server.wl_event_loop);

    server.backend = wlr_backend_autocreate(server.wl_display, NULL);
    assert(server.backend);

    wl_list_init(&server.outputs);
    server.new_output.notify = new_output_notify;
    wl_signal_add(&server.backend->events.new_output, &server.new_output);

    wl_list_init(&server.inputs);
    server.new_input.notify = new_input_notify;
    wl_signal_add(&server.backend->events.new_input, &server.new_input);

    const char *socket = wl_display_add_socket_auto(server.wl_display);
    assert(socket);

    if (!wlr_backend_start(server.backend)) {
        fprintf(stderr, "Failed to start backend.\n");
        wl_display_destroy(server.wl_display);
        return 1;
    }

    printf("Running compositor on display '%s'\n", socket);
    setenv("WAYLAND_DISPLAY", socket, true);

    wl_display_init_shm(server.wl_display);
    wlr_gtk_primary_selection_device_manager_create(server.wl_display);
    wlr_idle_create(server.wl_display);

    server.compositor = wlr_compositor_create(server.wl_display, wlr_backend_get_renderer(server.backend));

    wlr_xdg_shell_v6_create(server.wl_display);

    system("gnome-terminal -- htop &");

    wl_display_run(server.wl_display);
    wl_display_destroy(server.wl_display);

    return 0;
}
