include("${CMAKE_CURRENT_LIST_DIR}/FindWaylandProtocols.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/FindWaylandScanner.cmake")

message(STATUS "Using protocols directory: ${WAYLAND_PROTOCOLS_DIR}")

function(generate_wayland_protocol TYPE PROTOCOL_XML DESTINATION)
    execute_process(
            COMMAND ${WAYLAND_SCANNER_EXE} server-header ${PROTOCOL_XML} ${CMAKE_CURRENT_BINARY_DIR}/${DESTINATION}
    )
endfunction()

function(generate_wayland_protocol_header PROTOCOL_XML DESTINATION)
    execute_process(COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/protocol_header)
    generate_wayland_protocol(server-header ${PROTOCOL_XML} "protocol_header/${DESTINATION}")
endfunction()

function(generate_wayland_protocol_source PROTOCOL_XML DESTINATION)
    execute_process(COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/protocol_source)
    generate_wayland_protocol(private-code ${PROTOCOL_XML} "protocol_source/${DESTINATION}")
endfunction()

generate_wayland_protocol_header("${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml" "xdg-shell-protocol.h")
generate_wayland_protocol_source("${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml" "xdg-shell-protocol.c")

generate_wayland_protocol_header("${CMAKE_CURRENT_LIST_DIR}/../protocol/wlr-layer-shell-unstable-v1.xml" "wlr-layer-shell-unstable-v1-protocol.h")
generate_wayland_protocol_source("${CMAKE_CURRENT_LIST_DIR}/../protocol/wlr-layer-shell-unstable-v1.xml" "wlr-layer-shell-unstable-v1-protocol.c")
