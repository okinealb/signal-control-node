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

add_library(signal_control src/signal.c src/command.c)
target_include_directories(signal_control PUBLIC include)
target_compile_options(signal_control PRIVATE -Wall -Wextra -Wpedantic)

add_executable(signal_control_test tests/test_signal.c)
target_link_libraries(signal_control_test PRIVATE signal_control)
target_compile_options(signal_control_test PRIVATE -Wall -Wextra -Wpedantic)

add_executable(signal_command_test tests/test_command.c)
target_link_libraries(signal_command_test PRIVATE signal_control)
target_compile_options(signal_command_test PRIVATE -Wall -Wextra -Wpedantic)

add_executable(signal_node_test tests/test_node.c)
target_link_libraries(signal_node_test PRIVATE signal_control freertos_kernel)
target_compile_options(signal_node_test PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_test(NAME signal_control_test COMMAND signal_control_test)
add_test(NAME signal_command_test COMMAND signal_command_test)
add_test(NAME signal_node_test COMMAND signal_node_test)
