#
# LookAroundEyesApp - Dual Eye Tracking Comparison Application
#

cmake_minimum_required(VERSION 3.21)
project(LookAroundEyesApp)

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
    -DNOMINMAX
)

# LookAroundEyesApp executable
add_executable(LookAroundEyesApp
    WIN32
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/DualEyeTracker.cpp
    ${PROJECT_SOURCE_DIR}/src/DualEyeTracker.h
    ${PROJECT_SOURCE_DIR}/src/HUD.cpp
    ${PROJECT_SOURCE_DIR}/src/HUD.h
    ${PROJECT_SOURCE_DIR}/src/leia_math.h
    ${PROJECT_SOURCE_DIR}/src/Scene.cpp
    ${PROJECT_SOURCE_DIR}/src/Scene.h
)

target_include_directories(LookAroundEyesApp PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${LEIASR_SDKROOT}/include
    ${LEIASR_SDKROOT}/third_party/OpenCV/include
)

# Set C++17 standard
set_target_properties(LookAroundEyesApp PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

if (WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(BUILD_PLATFORM 32)
endif()

target_link_libraries(LookAroundEyesApp PRIVATE
    simulatedreality
    srDirectX${BUILD_PLATFORM}::srDirectX
    d3d11
    d3dcompiler
    dxgi
    shcore
    d2d1
    dwrite
)

# Copy SDK DLLs to output directory
add_custom_command(TARGET LookAroundEyesApp POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E copy_directory
    ${LEIASR_SDKROOT}/bin/x64
    $<TARGET_FILE_DIR:LookAroundEyesApp>
)
