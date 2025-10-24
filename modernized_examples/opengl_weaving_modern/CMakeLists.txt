#
# Copyright (C) 2025 Leia
# Modernized OpenGL Weaving Example with DisplayManager

cmake_minimum_required(VERSION 3.21)
project(example_opengl_weaving_modern)

set(LEIASR_SDKROOT "$ENV{LEIASR_SDKROOT}" CACHE PATH "Path to LEIASR SDK root")

# Tell CMake to look here for find_package()
list(APPEND CMAKE_PREFIX_PATH "${LEIASR_SDKROOT}/lib/cmake")

find_package(simulatedreality REQUIRED)
find_package(srOpenGL REQUIRED)

add_definitions(
    -DTW_STATIC
    -DTW_NO_LIB_PRAGMA
    -DTW_NO_DIRECT3D
    -D_CRT_SECURE_NO_WARNINGS
    -DSRDISPLAY_LAZYBINDING  # Enable lazy binding for version compatibility
)

# Opengl example
add_executable(example_opengl_weaving_modern
    WIN32
    ${PROJECT_SOURCE_DIR}/main.cpp
    ${PROJECT_SOURCE_DIR}/leia_math.h
    ${PROJECT_SOURCE_DIR}/resource.h
    ${PROJECT_SOURCE_DIR}/resource.rc
    ${PROJECT_SOURCE_DIR}/simple-opengl-loader.h
    ${PROJECT_SOURCE_DIR}/stb_image.h
    ${PROJECT_SOURCE_DIR}/StereoImage.jpg
    ${PROJECT_SOURCE_DIR}/targetver.h
    ${PROJECT_SOURCE_DIR}/icon_small.ico
    ${PROJECT_SOURCE_DIR}/icon_large.ico
    ${PROJECT_SOURCE_DIR}/../common/display_helper.h
)

target_include_directories(example_opengl_weaving_modern PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../common
)

if (WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(BUILD_PLATFORM 32)
endif()

target_link_libraries(example_opengl_weaving_modern PRIVATE
    simulatedreality
    srOpenGL${BUILD_PLATFORM}::srOpenGL
)

add_custom_command(
  TARGET example_opengl_weaving_modern POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${PROJECT_SOURCE_DIR}/StereoImage.jpg
          $<TARGET_FILE_DIR:example_opengl_weaving_modern>/StereoImage.jpg)

add_custom_command(TARGET example_opengl_weaving_modern POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE_DIR:example_opengl_weaving_modern> $<TARGET_RUNTIME_DLLS:example_opengl_weaving_modern>
    ${CMAKE_CURRENT_BINARY_DIR}
)
