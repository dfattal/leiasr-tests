#
# Copyright (C) 2025 Leia
#

cmake_minimum_required(VERSION 3.21)
project(example_parallax_toggle)

set(LEIASR_SDKROOT "$ENV{LEIASR_SDKROOT}" CACHE PATH "Path to LEIASR SDK root")

# Tell CMake to look here for find_package()
list(APPEND CMAKE_PREFIX_PATH "${LEIASR_SDKROOT}/lib/cmake")


find_package(simulatedreality REQUIRED)
find_package(srDirectX REQUIRED)

add_definitions(
    -DTW_STATIC
    -DTW_NO_LIB_PRAGMA
    -DTW_NO_DIRECT3D
    -DGLEW_STATIC
    -D_CRT_SECURE_NO_WARNINGS
)

# Parallax Toggle example
add_executable(example_parallax_toggle
    WIN32
    ${PROJECT_SOURCE_DIR}/main.cpp
    ${PROJECT_SOURCE_DIR}/leia_math.h
    ${PROJECT_SOURCE_DIR}/resource.h
    ${PROJECT_SOURCE_DIR}/resource.rc
    ${PROJECT_SOURCE_DIR}/targetver.h
    ${PROJECT_SOURCE_DIR}/icon_small.ico
    ${PROJECT_SOURCE_DIR}/icon_large.ico
)
target_include_directories(example_parallax_toggle PRIVATE
    ${PROJECT_SOURCE_DIR}
)

if (WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(BUILD_PLATFORM 32)
endif()

target_link_libraries(example_parallax_toggle PRIVATE
    simulatedreality
    srDirectX${BUILD_PLATFORM}::srDirectX
)

add_custom_command(TARGET example_parallax_toggle POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE_DIR:example_parallax_toggle> $<TARGET_RUNTIME_DLLS:example_parallax_toggle>
    ${CMAKE_CURRENT_BINARY_DIR}
)
