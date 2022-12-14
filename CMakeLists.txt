

cmake_minimum_required(VERSION 3.7.2)
project(sel4cp-workshop C)

# Create Header-Only libraries for sel4cp and sel4 to be linked to sel4cp C programs.
add_library(lib_sel4cp_qemu INTERFACE)
target_include_directories(lib_sel4cp_qemu INTERFACE ${CMAKE_CURRENT_LIST_DIR}/sdk/board/qemu_arm_virt/debug/include/)

add_subdirectory(workshop/)



