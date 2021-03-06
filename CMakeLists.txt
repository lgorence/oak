cmake_minimum_required(VERSION 3.16)
project(oak C)

find_package(PkgConfig REQUIRED)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/FindWlroots.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/FindPixman.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/FindXkbCommon.cmake")

set(SOURCES src/main.c src/input.c src/output.c src/view.c)

add_definitions(-DWLR_USE_UNSTABLE)

add_custom_target(wayland_protocols
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/Wayland.cmake)

if(INCEPTION)
    add_definitions(-DINCEPTION)
endif()

add_executable(oak ${SOURCES})
add_dependencies(oak wayland_protocols)
target_include_directories(oak PUBLIC include/ ${PIXMAN_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/protocol_header)
target_link_libraries(oak ${WLR_LIBRARIES} ${XKBCOMMON_LIBRARIES} ${PIXMAN_LIBRARIES} wayland-server)

install(TARGETS oak DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
