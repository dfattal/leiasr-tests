#
# LeiaSplatViewer - Gaussian Splat Viewer Application
#

cmake_minimum_required(VERSION 3.21)
project(LeiaSplatViewer)

set(LEIASR_SDKROOT "$ENV{LEIASR_SDKROOT}" CACHE PATH "Path to LEIASR SDK root")

# Fallback to relative path if environment variable not set
if(NOT LEIASR_SDKROOT OR LEIASR_SDKROOT STREQUAL "")
    set(LEIASR_SDKROOT "${CMAKE_CURRENT_SOURCE_DIR}/../simulatedreality-SDK-1.34.8.31132-win64-Release" CACHE PATH "Path to LEIASR SDK root" FORCE)
endif()

# Convert to absolute path
get_filename_component(LEIASR_SDKROOT "${LEIASR_SDKROOT}" ABSOLUTE)

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

# LeiaSplatViewer executable
add_executable(LeiaSplatViewer
    WIN32
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/OrbitCamera.cpp
    ${PROJECT_SOURCE_DIR}/src/OrbitCamera.h
    ${PROJECT_SOURCE_DIR}/src/SplatLoader.cpp
    ${PROJECT_SOURCE_DIR}/src/SplatLoader.h
    ${PROJECT_SOURCE_DIR}/src/leia_math.h
)

target_include_directories(LeiaSplatViewer PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${LEIASR_SDKROOT}/include
    ${LEIASR_SDKROOT}/third_party/OpenCV/include
)

# Set C++17 standard
set_target_properties(LeiaSplatViewer PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

if (WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(BUILD_PLATFORM 32)
endif()

target_link_libraries(LeiaSplatViewer PRIVATE
    simulatedreality
    srDirectX${BUILD_PLATFORM}::srDirectX
    d3d11
    d3dcompiler
)

# Copy SDK DLLs to output directory (if bin/x64 exists)
if(EXISTS "${LEIASR_SDKROOT}/bin/x64")
    add_custom_command(TARGET LeiaSplatViewer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying SDK DLLs from ${LEIASR_SDKROOT}/bin/x64"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${LEIASR_SDKROOT}/bin/x64"
            "$<TARGET_FILE_DIR:LeiaSplatViewer>"
        VERBATIM
    )
else()
    message(WARNING "SDK DLL directory not found: ${LEIASR_SDKROOT}/bin/x64 - DLLs will not be copied")
endif()
