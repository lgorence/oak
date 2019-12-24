find_program(WAYLAND_SCANNER_EXE wayland-scanner)

if(NOT WAYLAND_SCANNER_EXE)
    message(FATAL_ERROR "Failed to find wayland-scanner")
endif()
