
cmake_minimum_required(VERSION 3.7.2)

project(workshop C)

add_library(lib_client client.c)
target_include_directories(lib_client PUBLIC include)
target_link_libraries(
        lib_client
        lib_sel4cp_qemu
)

add_library(lib_serial_server serial_server.c)
target_include_directories(lib_serial_server PUBLIC include)
target_link_libraries(
        lib_serial_server
        lib_sel4cp_qemu
)

add_library(lib_wordle_server wordle_server.c)
target_include_directories(lib_wordle_server PUBLIC include)
target_link_libraries(
        lib_wordle_server
        lib_sel4cp_qemu
)
