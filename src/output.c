#include "output.h"
#include "view.h"

#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_surface.h>

void new_output_notify(struct wl_listener *listener, void *data) {
    struct oak_server *server = wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_output = data;

#ifdef INCEPTION
    wlr_output_set_custom_mode(wlr_output, 480, 960, 0);
#else
    if (!wl_list_empty(&wlr_output->modes)) {
        struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
        wlr_output_set_mode(wlr_output, mode);
        wlr_output_enable(wlr_output, true);
        if (!wlr_output_commit(wlr_output)) {
            fprintf(stderr, "Failed to commit\n");
            return;
        }
    }
#endif

    struct oak_output *output = calloc(1, sizeof(struct oak_output));
    clock_gettime(CLOCK_MONOTONIC, &output->last_frame);
    output->server = server;
    output->wlr_output = wlr_output;

    wl_list_insert(&server->outputs, &output->link);
    output->destroy.notify = output_destroy_notify;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);
    output->frame.notify = output_frame_notify;
    wl_signal_add(&wlr_output->events.frame, &output->frame);

    wlr_output_layout_add_auto(server->output_layout, wlr_output);

    wlr_output_create_global(wlr_output);

    // TODO: this probably belongs somewhere else.
    static bool launchedAcorn = false;
    if (!launchedAcorn && fork() == 0) {
        // sleeping allows something to propagate inside of wayland.
        // if we don't, the window we spawn has a chance of getting 1280x720 if in INCEPTION mode.
        sleep(1);
        execl("/bin/sh", "/bin/sh", "-c", "~/Dev/C++/Acorn/cmake-build-debug/main", (void*) NULL);
        launchedAcorn = true;
    }
}

void output_destroy_notify(struct wl_listener *listener, void *data) {
    struct oak_output *output = wl_container_of(listener, output, destroy);
    wl_list_remove(&output->link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->frame.link);
    free(output);
}

void output_frame_notify(struct wl_listener *listener, void *data) {
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

    struct oak_view *_view;
    wl_list_for_each_reverse(_view, &server->views, link) {
        struct wlr_surface *surface = _view->surface->surface;
        if (!wlr_surface_has_buffer(surface)) {
            continue;
        }
        struct wlr_box render_box = {
                .x = _view->x, .y = _view->y,
                .width = surface->current.width,
                .height = surface->current.height
        };
        float matrix[16];
        struct wlr_texture *texture = wlr_surface_get_texture(surface);
        wlr_matrix_project_box(&matrix, &render_box, surface->current.transform, 0, &wlr_output->transform_matrix);
        wlr_render_texture_with_matrix(renderer, texture, &matrix, 1.0f);
        wlr_surface_send_frame_done(surface, &now);
    }

    wlr_output_render_software_cursors(wlr_output, NULL);

    wlr_renderer_end(renderer);
    wlr_output_commit(wlr_output);
}
