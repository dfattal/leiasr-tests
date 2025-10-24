# LeiaSR SDK 1.34.8-RC1 Summary

## Overview

The **LeiaSR SDK** (Simulated Reality SDK) is a comprehensive development kit from Leia, Inc. for creating immersive 3D display applications and integrating advanced sensing capabilities. The SDK enables developers to build applications that leverage Leia's lightfield display technology, hand tracking, face tracking, and eye tracking features.

**Version**: 1.34.8-RC1 (Release Candidate)
**Platform**: Windows 64-bit
**License**: MIT License with 3rd party dependencies

## Core Capabilities

### 1. Display Management
- **Lightfield Display Control**: Support for Leia's 3D displays with automatic parameter configuration
- **View Weaving**: Multiple rendering backends for interlaced view generation:
  - OpenGL weaving
  - DirectX 9/10/11/12 weaving
- **Resolution and Display Parameters**: API for querying display characteristics (resolution, dimensions, etc.)

### 2. Tracking and Sensing

#### Face Tracking
- Real-time face detection and tracking
- Head pose estimation
- Eye tracking with gaze prediction
- Camera coordinate system integration

#### Hand Tracking
- Hand pose detection and tracking
- Palm position tracking
- Integration with UltraLeap hand tracking hardware
- Real-time hand event callbacks

#### System Sensing
- System event monitoring
- Application state management
- Display attachment/detachment detection

### 3. Camera Support
- Camera integration for tracking systems
- Coordinate system transformations
- Camera parameter management

## SDK Structure

```
LeiaSR-SDK-1.34.8-RC1-win64/
├── include/sr/          # C and C++ headers
│   ├── core_c.h         # Core context management (C API)
│   ├── displays_c.h     # Display management (C API)
│   ├── cameras_c.h      # Camera interfaces (C API)
│   ├── facetrackers_c.h # Face tracking (C API)
│   ├── handtrackers_c.h # Hand tracking (C API)
│   ├── sense/           # C++ sensing APIs
│   ├── weaver/          # View weaving APIs
│   └── management/      # Context and resource management
├── lib/                 # Static libraries
│   ├── simulatedreality.lib
│   ├── SimulatedRealityCore.lib
│   ├── SimulatedRealityDisplays.lib
│   ├── SimulatedRealityFaceTrackers.lib
│   ├── SimulatedRealityHandTrackers.lib
│   ├── SimulatedRealityOpenGL.lib
│   ├── SimulatedRealityDirectX.lib
│   └── cmake/           # CMake configuration files
├── examples/            # Sample applications
│   ├── c/               # C language examples
│   ├── cpp/             # C++ language examples
│   ├── csharp/          # C# examples
│   ├── opengl_weaving/  # OpenGL rendering example
│   ├── directx*_weaving/# DirectX rendering examples
│   ├── gesturerecognizer/
│   ├── lenshints/
│   ├── predictingeyetracker/
│   └── systemsense/
├── third_party/         # Dependencies
│   ├── asio/            # Async I/O library
│   ├── GLog/            # Google logging
│   ├── Leap/            # UltraLeap hand tracking
│   ├── OpenCV/          # Computer vision library
│   └── rapidxml/        # XML parsing
└── doc/                 # HTML documentation (Doxygen generated)
```

## Programming Languages Supported

- **C**: Full C API with callback-based interfaces
- **C++**: Object-oriented API with listeners and streams
- **C#**: .NET bindings for managed code

## Key APIs

### Core Context (C API)
```c
SRContext newSRContext();
void initializeSRContext(SRContext context);
void deleteSRContext(SRContext context);
```

### Display Management
```c
SR_display createDisplay(SRContext context);
const int getResolutionHeight(SR_display display);
const int getResolutionWidth(SR_display display);
```

### Hand Tracking (C++ API)
```cpp
SR::HandTracker* handTracker = SR::HandTracker::create(context);
handTracker->openHandPoseStream(&listener);
```

## Example Applications

1. **Basic C Example**: Demonstrates core context initialization and hand tracking
2. **Basic C++ Example**: Shows C++ API usage with hand pose listeners
3. **OpenGL Weaving**: Full rendering pipeline with view interlacing
4. **DirectX Weaving**: Multiple DirectX versions (9/10/11/12) support
5. **Gesture Recognizer**: Advanced hand gesture recognition
6. **Predicting Eye Tracker**: Eye tracking with gaze prediction
7. **System Sense**: System event monitoring
8. **Lens Hints**: Display lens control

## Build System

### CMake Integration
- **Minimum CMake Version**: 3.12 (most examples), 3.21 (OpenGL example)
- **Package Management**: Uses `find_package(simulatedreality REQUIRED)`
- **Environment Variable**: `LEIASR_SDKROOT` must be set to SDK root directory
- **Installation Script**: `Install.bat` automatically sets environment variable

### Typical CMake Usage
```cmake
find_package(simulatedreality REQUIRED)
add_executable(my_app main.cpp)
target_link_libraries(my_app simulatedreality)
```

## Third-Party Dependencies

- **ASIO**: Asynchronous I/O (Boost Software License)
- **Boost**: General purpose C++ libraries (Boost Software License)
- **GLog**: Google logging library (BSD 3-Clause)
- **UltraLeap**: Hand tracking SDK (Ultraleap SDK Agreement)
- **OpenCV**: Computer vision library (BSD 3-Clause)
- **rapidxml**: XML parsing library (Boost/MIT dual license)

## Installation Requirements

1. Extract SDK to desired location
2. Set `LEIASR_SDKROOT` environment variable (use `Install.bat` or set manually)
3. Configure CMake with SDK path: `CMAKE_PREFIX_PATH="${LEIASR_SDKROOT}/lib/cmake"`

## Target Platform

- **Operating System**: Windows (Win32)
- **Architecture**: 64-bit (with some 32-bit support for specific components)
- **Build Tools**: CMake 3.12+, Visual Studio (for Windows builds)

## Use Cases

1. **3D Gaming**: Render games with lightfield display support
2. **Medical Visualization**: 3D medical imaging applications
3. **CAD/3D Modeling**: Enhanced 3D visualization for design tools
4. **Interactive Kiosks**: Touchless hand-controlled interfaces
5. **Virtual Collaboration**: Eye-tracked 3D video conferencing
6. **Educational Software**: Immersive 3D learning experiences

## Documentation

- Comprehensive HTML documentation (Doxygen generated) in `/doc` directory
- Architecture guides
- Application guidelines
- API reference with code examples
- Coordinate system diagrams

## Architecture

The SDK follows a modular architecture:

1. **Core Layer**: Context management, initialization, and lifecycle
2. **Sense Layer**: Input streams for tracking data (hands, face, eyes)
3. **Display Layer**: Display parameter access and management
4. **Weaver Layer**: View interlacing for multiple rendering backends
5. **Utility Layer**: Helper functions and common utilities

## Development Workflow

1. Create `SRContext` to manage SDK lifecycle
2. Instantiate required senses (display, hand tracker, face tracker, etc.)
3. Register listeners/callbacks for tracking data
4. Initialize context (starts all senses)
5. Process tracking data in callbacks
6. Render content with view weaving
7. Clean up context on exit

## Notes

- This is a **Release Candidate (RC1)** version, indicating pre-release status
- Windows-focused SDK (WIN32 preprocessor directives throughout)
- Requires display drivers and potentially specialized Leia display hardware
- Some examples may require specific DirectX or OpenGL versions
