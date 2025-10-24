# Modernized SDK Examples with DisplayManager

## Overview

This directory contains modernized versions of LeiaSR SDK examples that use the new **IDisplayManager** interface with automatic fallback to the legacy Display API. These examples demonstrate version-safe display access that works across different runtime versions.

## What's New

### DisplayAccess Helper Class

Location: `modernized_examples/common/display_helper.h`

A unified wrapper class that provides:
- **Automatic API detection**: Uses `TryGetDisplayManagerInstance()` to detect modern API
- **Transparent fallback**: Falls back to legacy `Display` class if modern API unavailable
- **Unified interface**: Same API regardless of underlying implementation
- **Version safety**: `isValid()` checks and safe default values
- **Built-in wait**: `waitForDisplay()` method with timeout

### Key Features

1. **Lazy Binding**: Uses `SRDISPLAY_LAZYBINDING` preprocessor macro
2. **No Hard Dependencies**: Works with older runtimes that don't have IDisplayManager
3. **Graceful Degradation**: Provides sensible defaults when display not found
4. **Runtime Detection**: Logs whether modern or legacy API is in use

## Examples Included

### 1. cpp_modern

**Location**: `modernized_examples/cpp_modern/`

Enhanced C++ example that demonstrates:
- Display information retrieval using modern API
- Display validity checking
- Default viewing position (modern API only feature)
- API detection reporting
- Hand tracking integration

**Key Changes**:
```cpp
// Old:
SR::SRContext context;
SR::HandTracker* handTracker = SR::HandTracker::create(context);

// New:
SR::SRContext context;
SR::Helper::DisplayAccess displayAccess(context);

if (displayAccess.isDisplayValid()) {
    std::cout << "Display: " << displayAccess.getResolutionWidth()
              << "x" << displayAccess.getResolutionHeight() << std::endl;

    // Modern API feature:
    float x, y, z;
    if (displayAccess.getDefaultViewingPosition(x, y, z)) {
        std::cout << "Viewing position: " << x << ", " << y << ", " << z << std::endl;
    }
}
```

### 2. opengl_weaving_modern

**Location**: `modernized_examples/opengl_weaving_modern/`

Modernized OpenGL weaving example with:
- DisplayAccess for window positioning
- Simplified display readiness checking
- Version-safe texture size queries

**Key Changes**:
```cpp
// Old: Multiple Display::create() calls with manual validity checks
SR::Display* display = SR::Display::create(*g_srContext);
SR_recti displayLocation = display->getLocation();

// New: Single DisplayAccess instance with built-in validation
SR::Helper::DisplayAccess displayAccess(*g_srContext);
SR_recti displayLocation = displayAccess.getLocation();

// Old: Manual polling loop with timeout
while (!displayReady) {
    SR::Display* display = SR::Display::create(*g_srContext);
    // ... manual validity checking ...
}

// New: Built-in wait with timeout
displayReady = displayAccess.waitForDisplay(maxTime);
```

## Build System

### GitHub Actions Workflow

**File**: `.github/workflows/build-modernized-examples.yml`

- Builds both modernized examples on Windows runners
- Uses CMake 3.21+
- Automatically sets up SDK environment
- Uploads artifacts to `new_examples/` directory

### Local Build

```bash
# Windows
set LEIASR_SDKROOT=C:\path\to\LeiaSR-SDK-1.34.8-RC1-win64

cd modernized_examples/cpp_modern
cmake -S . -B build -DCMAKE_PREFIX_PATH="%LEIASR_SDKROOT%/lib/cmake"
cmake --build build --config Release

cd ../opengl_weaving_modern
cmake -S . -B build -DCMAKE_PREFIX_PATH="%LEIASR_SDKROOT%/lib/cmake"
cmake --build build --config Release
```

## Built Executables

Downloaded artifacts are in: `/Users/david.fattal/Documents/GitHub/leiasr-tests/new_examples/`

- `example_cpp_modern.exe` (20K) - Enhanced C++ example with display info
- `example_opengl_weaving_modern.exe` (263K) - Modernized OpenGL rendering

## Comparison: Legacy vs Modern

### Legacy Approach (Original Examples)

```cpp
// Assumes display always exists
SR::Display* display = SR::Display::create(context);
int width = display->getResolutionWidth();  // May fail if no display

// Manual validity checking
SR_recti location = display->getLocation();
if (location.right - location.left == 0) {
    // No display detected
}

// No version compatibility checking
// Crashes if runtime doesn't support API
```

### Modern Approach (Modernized Examples)

```cpp
// Automatic version detection
SR::Helper::DisplayAccess display(context);

// Built-in validity checking
if (display.isDisplayValid()) {
    int width = display.getResolutionWidth();
}

// Version-safe features
float x, y, z;
if (display.getDefaultViewingPosition(x, y, z)) {
    // Modern API feature available
} else {
    // Fallback behavior for legacy API
}

// Works with both old and new runtimes
std::cout << "API: " << (display.isUsingModernAPI() ? "Modern" : "Legacy") << std::endl;
```

## Migration Guide

### Step 1: Add Display Helper

```cpp
#define SRDISPLAY_LAZYBINDING  // Before including display.h
#include "sr/world/display/display.h"
#include "../common/display_helper.h"
```

### Step 2: Replace Display::create() Calls

```cpp
// Old:
SR::Display* display = SR::Display::create(context);
int width = display->getResolutionWidth();

// New:
SR::Helper::DisplayAccess display(context);
int width = display.getResolutionWidth();
```

### Step 3: Add Validity Checks

```cpp
if (display.isDisplayValid()) {
    // Use display parameters
} else {
    // Handle no-display case
}
```

### Step 4: Use Modern Features (Optional)

```cpp
float x, y, z;
if (display.getDefaultViewingPosition(x, y, z)) {
    // Only available in modern API
}
```

### Step 5: Update CMakeLists.txt

```cmake
add_definitions(-DSRDISPLAY_LAZYBINDING)

target_include_directories(my_app PRIVATE
    ${PROJECT_SOURCE_DIR}/../common  # For display_helper.h
)
```

## Benefits

### For Developers
- ✅ Single unified API for display access
- ✅ Works across SDK versions
- ✅ No crashes from missing displays
- ✅ Clear logging of API in use

### For Users
- ✅ Applications work on older runtimes
- ✅ Graceful degradation when display not found
- ✅ Better error messages
- ✅ Forward compatibility with new features

### For Testing
- ✅ Examples build successfully on CI
- ✅ No runtime dependencies on specific SDK versions
- ✅ Easy to test with/without SR display


## Next Steps

To modernize more examples:

1. Copy example directory to `modernized_examples/<name>_modern/`
2. Include `display_helper.h` and define `SRDISPLAY_LAZYBINDING`
3. Replace `Display::create()` with `DisplayAccess`
4. Add validity checks with `isDisplayValid()`
5. Update CMakeLists.txt with `-DSRDISPLAY_LAZYBINDING`
6. Add to matrix in `.github/workflows/build-modernized-examples.yml`
