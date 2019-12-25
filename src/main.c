#include "server.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <wlr/backend.h>
#include <wlr/types/wlr_gtk_primary_selection.h>
#include <wlr/types/wlr_idle.h>
#include <wlr/types/wlr_xdg_shell_v6.h>

#include "input.h"
#include "output.h"

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
