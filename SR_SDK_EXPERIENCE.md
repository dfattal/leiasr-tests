# Leia SR SDK - Developer Experience Report

**SDK Version**: 1.34.8.31132-win64-Release
**Date**: October 2025
**Context**: Building C++ DirectX 11 applications with CMake on GitHub Actions

---

## What Worked Well ✅

### 1. **CMake Integration**
- The SDK provides proper CMake config files in `lib/cmake/`
- `find_package(simulatedreality REQUIRED)` works seamlessly
- `find_package(srDirectX REQUIRED)` integrates DirectX weaver cleanly
- CMake targets are well-structured with proper include directories and link libraries

### 2. **Header Organization**
- Once you know the correct paths, header organization is logical:
  - `sr/management/` - Core context and configuration
  - `sr/world/display/` - Display abstraction
  - `sr/weaver/` - Graphics API integration (DX9/10/11/12, OpenGL)
  - `sr/sense/` - Tracking and sensing subsystems
  - `sr/utility/` - Common utilities and exceptions
- Clear separation between C API (`*_c.h`) and C++ API

### 3. **API Design**
- Modern C++ interfaces with proper inheritance (`IWeaverBase1`, `IDisplay`)
- Clean resource management with `IDestroyable` interface
- Exception-based error handling with specific exception types (`ServerNotAvailableException`)
- Type-safe `IQueryInterface` for capability detection

### 4. **DirectX 11 Weaver API**
- `CreateDX11Weaver()` has a clean signature with proper error codes
- `setInputViewTexture()` accepts standard D3D11 shader resource views
- `getPredictedEyePositions()` for latency compensation is well-designed
- Latency management via `setLatency()` and `setLatencyInFrames()` is thoughtful

### 5. **Build System**
- DLL post-build copy commands work correctly
- SDK bin directory structure (`bin/x64/`) is consistent
- Third-party dependencies (OpenCV, asio, GLog) are bundled and working

---

## What Could Be Improved 📋

### 1. **Header Path Documentation** ⚠️ **HIGH PRIORITY**

**Issue**: The actual header paths don't match what you might expect from SDK examples or migration guides.

**Specific Problems**:
- No clear documentation stating that `sr/context.h` doesn't exist
- The correct path is `sr/management/srcontext.h`, but this isn't obvious
- Similarly, display functionality requires `sr/world/display/display.h`, not an intuitive "main" header

**Recommendation**:
```cpp
// Add a "Getting Started" header reference guide:
// CORRECT HEADERS FOR SDK v1.34.8+
#include "sr/management/srcontext.h"     // For SR::SRContext
#include "sr/world/display/display.h"    // For SR::IDisplay, GetMainSRDisplay()
#include "sr/weaver/dx11weaver.h"        // For SR::CreateDX11Weaver(), IDX11Weaver1
#include "sr/utility/exception.h"        // For SR::ServerNotAvailableException
```

### 2. **API Migration Guide** ⚠️ **HIGH PRIORITY**

**Issue**: No clear migration documentation for deprecated APIs.

**Specific Problems**:
- Old API: `SR::dx11WeaverCreate()` → New API: `SR::CreateDX11Weaver()`
- Old API: `weaver->setSourceViews()` → New API: `weaver->setInputViewTexture()`
- Old API: `weaver->release()` → New API: `weaver->destroy()`
- Old API: `context->release()` → New API: `SR::SRContext::deleteSRContext(context)`

**Recommendation**: Add a `MIGRATION.md` or `CHANGELOG_API.md` documenting:
- What methods were removed/renamed in each version
- Side-by-side code examples of old vs new
- Why the changes were made (helps developers understand the pattern)

### 3. **Example Code Coverage** ⚠️ **MEDIUM PRIORITY**

**Issue**: The SDK example (`directx11_weaving`) uses the current API correctly, but there's only one DirectX 11 example.

**What Would Help**:
- A minimal "Hello Leia" example showing just context + display + weaver (< 200 lines)
- An example showing proper exception handling patterns
- An example demonstrating `IDisplay::getRecommendedViewsTextureWidth()` usage
- Examples for common tasks:
  - Setting up side-by-side stereo rendering
  - Handling display disconnection/reconnection
  - Late latching for low-latency rendering

### 4. **Error Messages and Debugging** ⚠️ **MEDIUM PRIORITY**

**Issue**: When headers are wrong, you get C++ compiler errors that don't point to the root cause.

**Example**:
```
error C1083: Cannot open include file: 'sr/context.h': No such file or directory
```

**What Would Help**:
- A `sr/common.h` or `sr/sr.h` "master header" that includes the most common headers
- Compile-time warnings when deprecated patterns are detected (if possible with pragmas)
- A troubleshooting section in docs: "Common Compiler Errors and Solutions"

### 5. **CMake Find Module Documentation** ⚠️ **LOW PRIORITY**

**Issue**: The CMake integration works, but it's not documented how it was discovered.

**What Would Help**:
```cmake
# Document what the find_package provides:
# find_package(simulatedreality REQUIRED)
#   Provides: simulatedreality target
#   Include directories: Automatically set via target_link_libraries
#   Library: simulatedreality.lib
#   DLLs: Located in ${LEIASR_SDKROOT}/bin/x64/

# find_package(srDirectX REQUIRED)
#   Provides: srDirectX${BUILD_PLATFORM}::srDirectX target
#   BUILD_PLATFORM: Empty for x64, "32" for x86
```

### 6. **Display Interface Clarity** ⚠️ **LOW PRIORITY**

**Issue**: Multiple display-related interfaces without clear hierarchy documentation.

**Observed Interfaces**:
- `SR::Display` (class in display.h, line 31)
- `SR::IDisplay` (interface in display.h, line 122)
- `GetMainSRDisplay()` returns `SR::IDisplay*`

**What Would Help**:
- Clarify when to use `Display::create()` vs `GetMainSRDisplay()`
- Document the relationship between these types
- Explain if `IDisplay` is the preferred interface going forward

---

## Summary

The Leia SR SDK is **well-architected** with modern C++ patterns, clean CMake integration, and a thoughtful API design. The core functionality works reliably once you know the correct patterns.

The main friction points are:
1. **Discoverability** - Finding the correct header paths requires trial and error
2. **Migration** - No clear guidance for moving from old to new API patterns
3. **Examples** - Limited variety of example code for common scenarios

### Recommended Quick Wins:
1. Add a "Quick Start - Correct Headers" section to the main README
2. Create an API migration guide showing old → new patterns
3. Add 2-3 more minimal examples demonstrating common use cases

### SDK Rating:
- **Functionality**: ⭐⭐⭐⭐⭐ (5/5) - Works great once configured
- **Documentation**: ⭐⭐⭐☆☆ (3/5) - Needs header path and migration docs
- **Developer Experience**: ⭐⭐⭐⭐☆ (4/5) - Clean API, but discovery friction

---

## Positive Highlights

These aspects of the SDK deserve special recognition:

1. **Exception Safety**: The SDK uses proper C++ exceptions instead of error codes, making error handling natural
2. **Late Latching Support**: The prediction and latency compensation features show deep understanding of real-time 3D rendering
3. **Graphics API Agnostic**: Supporting DX9/10/11/12 and OpenGL with a consistent API is impressive
4. **Resource Management**: Clear ownership semantics with `create()`/`destroy()` patterns
5. **Type Safety**: Using `IQueryInterface` for capability detection is a smart pattern

The SDK team clearly has strong C++ expertise and understands real-time graphics pipelines well. With improved discoverability documentation, this would be an excellent SDK.
