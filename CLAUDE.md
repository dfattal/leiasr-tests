# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository Overview

This is a test repository for the **LeiaSR SDK 1.34.8-RC1** (Simulated Reality SDK) from Leia, Inc. The repository contains the complete SDK and builds all example applications via GitHub Actions. The SDK provides APIs for lightfield 3D displays, hand tracking, face tracking, and eye tracking on Windows.

## SDK Architecture

The LeiaSR SDK is committed directly into this repository at `LeiaSR-SDK-1.34.8-RC1-win64/` and follows this structure:

- **Core Context Management**: All SDK operations require creating an `SRContext` first, then initializing it after all senses are attached
- **Sense-Based Architecture**: Modular components (Display, HandTracker, FaceTracker, Camera) that attach to the context
- **Listener Pattern**: C++ API uses listener classes (e.g., `HandPoseListener`) with `accept()` callbacks for streaming data
- **Dual API**: Both C and C++ APIs available, C API uses function pointers for callbacks, C++ uses virtual classes
- **View Weaving**: Separate modules for OpenGL and DirectX (9/10/11/12) interlaced rendering for lightfield displays

## Building SDK Examples

All 12 SDK examples are built automatically on every push via GitHub Actions. To build locally:

```bash
# Set SDK root environment variable (Windows)
set LEIASR_SDKROOT=C:\path\to\leiasr-tests\LeiaSR-SDK-1.34.8-RC1-win64

# Configure with CMake (from SDK example directory)
cd LeiaSR-SDK-1.34.8-RC1-win64/examples/cpp
cmake -S . -B build -DCMAKE_PREFIX_PATH="%LEIASR_SDKROOT%/lib/cmake"

# Build
cmake --build build --config Release
```

### GitHub Actions Workflow

The workflow at `.github/workflows/build-leiasr-examples.yml` builds all examples on Windows runners:
- Uses CMake 3.21+
- Builds 11 C/C++ examples in parallel matrix jobs
- Separate job for C# example (requires .NET Framework 4.6.1 Developer Pack)
- Artifacts uploaded with 7-day retention to `examples/` directory structure

To download build artifacts:
```bash
gh run download <run-id> -D ./examples
```

## CMake Package System

The SDK uses CMake package configs at `lib/cmake/`. Key points:

- **Package names**: `simulatedreality`, `srOpenGL`, `srDirectX`
- The `simulatedreality` package automatically links all dependencies including:
  - All SimulatedReality*.lib files
  - Third-party libraries: GLog, OpenCV, LeapC (UltraLeap)
- Third-party x64/x86 libraries ARE committed to git (exception in `.gitignore` lines 29-31)
- Example CMakeLists.txt pattern:
  ```cmake
  find_package(simulatedreality REQUIRED)
  add_executable(my_app main.cpp)
  target_link_libraries(my_app simulatedreality)
  ```

## Critical .gitignore Configuration

**IMPORTANT**: The repository has a specific `.gitignore` pattern that allows SDK libraries while excluding build outputs:

- Lines 18-27: Excludes all `x64/` and `x86/` directories (standard build outputs)
- Lines 29-31: **EXCEPTION** for `!LeiaSR-SDK-*/third_party/**/x64/` and `x86/`
- This allows GLog and OpenCV libraries to be committed while preventing build artifact commits

When adding new SDK versions, ensure third-party libraries in `x64/` and `x86/` are force-added:
```bash
git add -f LeiaSR-SDK-*/third_party/*/lib/x64/
git add -f LeiaSR-SDK-*/third_party/*/lib/x86/
```

## SDK Examples Overview

12 examples demonstrate different SDK capabilities:

**Basic Examples:**
- `c/` - C API with hand tracking callbacks
- `cpp/` - C++ API with hand pose listener pattern
- `csharp/` - .NET Framework 4.6.1 managed code example

**Rendering Examples:**
- `opengl_weaving/` - OpenGL interlaced view rendering
- `directx9_weaving/` through `directx12_weaving/` - DirectX rendering variants

**Tracking Examples:**
- `gesturerecognizer/` - Hand gesture recognition
- `predictingeyetracker/` - Eye tracking with gaze prediction
- `systemsense/` - System event monitoring
- `lenshints/` - Display lens control

## Development Workflow

The typical SDK context lifecycle in examples:
1. Create `SRContext` via `newSRContext()` (C) or `SR::SRContext` (C++)
2. Create senses (e.g., `createHandTracker()`)
3. Open streams and attach listeners
4. Call `initializeSRContext()` - this starts all senses
5. Process data in callbacks/listeners
6. Clean up context on exit

## Reference Documentation

- `sdk-summary.md` - Comprehensive SDK feature overview, API examples, and architecture
- `LeiaSR-SDK-1.34.8-RC1-win64/doc/` - Full Doxygen HTML documentation (1500+ files)
- `LeiaSR-SDK-1.34.8-RC1-win64/License.txt` - MIT license with third-party attributions
