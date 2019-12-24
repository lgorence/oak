include("${CMAKE_CURRENT_LIST_DIR}/FindWaylandProtocols.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/FindWaylandScanner.cmake")

message(STATUS "Using protocols directory: ${WAYLAND_PROTOCOLS_DIR}")

function(generate_wayland_protocol TYPE PROTOCOL_XML_RELATIVE DESTINATION)
    execute_process(
            COMMAND ${WAYLAND_SCANNER_EXE} server-header ${WAYLAND_PROTOCOLS_DIR}/${PROTOCOL_XML_RELATIVE} ${CMAKE_CURRENT_BINARY_DIR}/${DESTINATION}
    )
endfunction()

function(generate_wayland_protocol_header PROTOCOL_XML_RELATIVE DESTINATION)
    execute_process(COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/protocol_header)
    generate_wayland_protocol(server-header ${PROTOCOL_XML_RELATIVE} "protocol_header/${DESTINATION}")
endfunction()

function(generate_wayland_protocol_source PROTOCOL_XML_RELATIVE DESTINATION)
    execute_process(COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/protocol_source)
    generate_wayland_protocol(private-code ${PROTOCOL_XML_RELATIVE} "protocol_source/${DESTINATION}")
endfunction()

generate_wayland_protocol_header("unstable/xdg-shell/xdg-shell-unstable-v6.xml" "xdg-shell-unstable-v6-protocol.h")
generate_wayland_protocol_source("unstable/xdg-shell/xdg-shell-unstable-v6.xml" "xdg-shell-unstable-v6-protocol.c")
