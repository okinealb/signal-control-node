cmake_minimum_required(VERSION 3.23)

project(signal_control_node LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(FetchContent)

add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/config)

set(FREERTOS_PORT "GCC_POSIX" CACHE STRING "" FORCE)

FetchContent_Declare(
  freertos_kernel
  GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
  GIT_TAG ce221a8bb468e462ca6b435cef66a9636e00baf4)
FetchContent_MakeAvailable(freertos_kernel)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND FREERTOS_PORT STREQUAL "GCC_POSIX")
  target_compile_definitions(freertos_kernel_port PRIVATE _POSIX_C_SOURCE=200809L)
endif()

add_library(sigctl src/signal.c)
target_include_directories(sigctl PUBLIC include)
target_compile_options(sigctl PRIVATE -Wall -Wextra -Wpedantic)

add_library(sigctl_node src/command.c src/control.c)
target_include_directories(sigctl_node PUBLIC include)
target_link_libraries(sigctl_node PUBLIC sigctl)
target_compile_options(sigctl_node PRIVATE -Wall -Wextra -Wpedantic)

add_executable(signal_control_test tests/test_signal.c)
target_link_libraries(signal_control_test PRIVATE sigctl)
target_compile_options(signal_control_test PRIVATE -Wall -Wextra -Wpedantic
                       -UNDEBUG)

add_executable(signal_command_test tests/test_command.c)
target_link_libraries(signal_command_test PRIVATE sigctl_node)
target_compile_options(signal_command_test PRIVATE -Wall -Wextra -Wpedantic
                       -UNDEBUG)

add_executable(signal_node_test tests/test_node.c)
target_link_libraries(signal_node_test PRIVATE sigctl_node freertos_kernel)
target_compile_options(signal_node_test PRIVATE -Wall -Wextra -Wpedantic
                       -UNDEBUG)

enable_testing()
add_test(NAME signal_control_test COMMAND signal_control_test)
add_test(NAME signal_command_test COMMAND signal_command_test)
add_test(NAME signal_node_test COMMAND signal_node_test)
set_tests_properties(signal_control_test signal_command_test signal_node_test
                     PROPERTIES TIMEOUT 5)
