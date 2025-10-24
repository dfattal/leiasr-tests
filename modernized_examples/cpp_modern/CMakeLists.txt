#
# Copyright (C) 2025 Leia, Inc.
# Modernized C++ Example with DisplayManager
#

cmake_minimum_required(VERSION 3.12)
project(example_cpp_modern)

# Enable lazy binding for DisplayManager
add_definitions(-DSRDISPLAY_LAZYBINDING)

find_package(simulatedreality REQUIRED)

add_executable(example_cpp_modern ${PROJECT_SOURCE_DIR}/src/main.cpp)

target_include_directories(example_cpp_modern PRIVATE
    ${PROJECT_SOURCE_DIR}/..
)

target_link_libraries(example_cpp_modern simulatedreality)
