cmake_minimum_required(VERSION 3.23)

project(signal_control_node LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(signal_control src/signal.c)
target_include_directories(signal_control PUBLIC include)
target_compile_options(signal_control PRIVATE -Wall -Wextra -Wpedantic)

add_executable(signal_control_test tests/test_signal.c)
target_link_libraries(signal_control_test PRIVATE signal_control)
target_compile_options(signal_control_test PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_test(NAME signal_control_test COMMAND signal_control_test)
