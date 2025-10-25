# DisplayManager Investigation Report

## Overview

LeiaSR SDK 1.34.8-RC1 introduces a new **DisplayManager** architecture that provides improved version compatibility and graceful degradation when interfacing with older runtime versions. This report documents the new interfaces, their design patterns, and the lazy binding mechanism that prevents runtime errors.

## Architecture Components

### 1. IDisplayManager Interface

**Location**: `LeiaSR-SDK-1.34.8-RC1-win64/include/sr/world/display/display.h:236-256`

The `IDisplayManager` is the primary interface for managing SR displays in this SDK version.

```cpp
class IDisplayManager : public virtual SR::IQueryInterface
{
public:
    virtual IDisplay* getPrimaryActiveSRDisplay() = 0;
    static std::unique_ptr<IDisplayManager> create(SR::SRContext& ctx);

protected:
    IDisplayManager() = default;
    virtual ~IDisplayManager() = default;
    // Non-copyable, non-movable
};
```

**Key Features**:
- Inherits from `IQueryInterface` enabling runtime interface discovery
- Factory pattern via `create()` static method
- Returns `IDisplay*` pointers (not owned by caller, context-owned)
- Non-copyable and non-movable design (deleted copy/move constructors)

### 2. IDisplay Interface

**Location**: `LeiaSR-SDK-1.34.8-RC1-win64/include/sr/world/display/display.h:127-229`

The `IDisplay` interface represents an individual SR display with version-safe query capabilities.

```cpp
class IDisplay : public virtual SR::IQueryInterface
{
public:
    // Version compatibility check
    virtual bool isValid() const = 0;
    virtual uint64_t identifier() const = 0;

    // Display parameters
    virtual int getResolutionHeight() const = 0;
    virtual int getResolutionWidth() const = 0;
    virtual int getPhysicalResolutionHeight() const = 0;
    virtual int getPhysicalResolutionWidth() const = 0;
    virtual float getPhysicalSizeHeight() const = 0;
    virtual float getPhysicalSizeWidth() const = 0;
    virtual float getDotPitch() const = 0;
    virtual SR_recti getLocation() = 0;

    // Rendering recommendations
    virtual int getRecommendedViewsTextureWidth() const = 0;
    virtual int getRecommendedViewsTextureHeight() const = 0;
    virtual void getDefaultViewingPosition(float& x_mm, float& y_mm, float& z_mm) const = 0;
};
```

**Key Features**:
- `isValid()` returns false when no SR display is found or parameters are invalid
- When `isValid() == false`, methods return safe default values instead of throwing errors
- `identifier()` returns a unique ID even when display is invalid (for tracking purposes)
- All getter methods are `const` for thread safety
- Includes new method `getDefaultViewingPosition()` not present in legacy `Display` class

### 3. Legacy Display Class (Deprecated)

**Location**: `LeiaSR-SDK-1.34.8-RC1-win64/include/sr/world/display/display.h:31-119`

The original `Display` class is maintained for backward compatibility but uses pure virtual methods:

```cpp
class DIMENCOSR_API Display : public WorldObject {
public:
    static Display* create(SRContext &context);
    virtual ~Display() = default;

    virtual const int getResolutionHeight() = 0;
    virtual const int getResolutionWidth() = 0;
    // ... other virtual methods
};
```

**Differences from IDisplay**:
- No `isValid()` check - assumes display always exists
- Non-const methods (older design)
- Missing `getDefaultViewingPosition()` method
- Part of the `WorldObject` hierarchy

### 4. Screen Class (Fully Deprecated)

**Location**: `LeiaSR-SDK-1.34.8-RC1-win64/include/sr/world/display/screen.h:34`

```cpp
class [[deprecated("Use Display class instead.")]] DIMENCOSR_API Screen : public WorldObject
```

The `Screen` class is marked deprecated with C++14 attributes and should not be used in new code.

## Version Compatibility Mechanism

### Problem Statement

When SDK code built against version 1.34.8-RC1 runs with an older runtime that doesn't include `IDisplayManager`, hard-linking would cause:
1. **Load-time errors**: DLL fails to load due to missing export
2. **Application crashes**: Before any error handling can occur
3. **Poor user experience**: No graceful degradation

### Solution: Lazy Binding

The SDK provides an **optional lazy binding mechanism** activated by defining `SRDISPLAY_LAZYBINDING` preprocessor macro.

#### Access Methods

**1. GetDisplayManagerInstance() - Direct Access**

**Location**: `display.h:265`

```cpp
extern "C" DIMENCOSR_API SR::IDisplayManager* GetDisplayManagerInstance(SR::SRContext& context);
```

- Standard exported C function
- **Throws error** if runtime doesn't support DisplayManager
- Use when you require DisplayManager functionality
- Suitable for new applications targeting 1.34.8-RC1+ runtime

**2. TryGetDisplayManagerInstance() - Safe Access**

**Location**: `display.h:290-306`

```cpp
inline SR::IDisplayManager* TryGetDisplayManagerInstance(SRContext& context)
{
    using FnType = SR::IDisplayManager* (*)(SRContext& context);

    // Static cache of resolved address
    static FnType fn = []() -> FnType {
        HMODULE hMod = GetModuleHandleA("SimulatedRealityDisplays");
        if (!hMod) hMod = GetModuleHandleA("SimulatedRealityDisplaysd");      // Debug build
        if (!hMod) hMod = GetModuleHandleA("SimulatedRealityDisplays32");     // 32-bit
        if (!hMod) hMod = GetModuleHandleA("SimulatedRealityDisplays32d");    // 32-bit debug
        if (!hMod) return nullptr;
        auto func = reinterpret_cast<FnType>(GetProcAddress(hMod, "GetDisplayManagerInstance"));
        return func;
    }();

    return fn ? fn(context) : nullptr;
}
```

**How It Works**:

1. **Lazy Resolution**: Uses lambda static initializer (executed once on first call)
2. **Runtime Search**: Queries already-loaded DLLs via `GetModuleHandleA()` (no new DLL loading)
3. **Symbol Resolution**: Uses `GetProcAddress()` to find `GetDisplayManagerInstance` export
4. **Caching**: Function pointer cached in static variable for performance
5. **Graceful Failure**: Returns `nullptr` if interface doesn't exist

**Benefits**:
- **No hard dependency**: Compiles and links even if symbol doesn't exist at runtime
- **Safe degradation**: Returns `nullptr` instead of crashing
- **Zero overhead**: Static initialization happens once, subsequent calls are direct pointer invocations
- **Multi-version support**: Checks multiple DLL variants (release/debug, x86/x64)

### Usage Pattern

```cpp
#define SRDISPLAY_LAZYBINDING  // Enable lazy binding
#include "sr/world/display/display.h"

// In your code:
SR::IDisplayManager* displayMgr = SR::TryGetDisplayManagerInstance(context);
if (displayMgr) {
    // Runtime supports new interface
    SR::IDisplay* display = displayMgr->getPrimaryActiveSRDisplay();
    if (display->isValid()) {
        // SR display found and configured
        int width = display->getResolutionWidth();
    } else {
        // No SR display, but interface exists - use defaults
    }
} else {
    // Older runtime - fall back to legacy Display class
    SR::Display* legacyDisplay = SR::Display::create(context);
    int width = legacyDisplay->getResolutionWidth();
}
```

## IQueryInterface Pattern

**Location**: `LeiaSR-SDK-1.34.8-RC1-win64/include/sr/utility/commoninterfaces.h:28-39`

Both `IDisplayManager` and `IDisplay` inherit from `IQueryInterface`:

```cpp
class IQueryInterface
{
protected:
    virtual void* queryInterface(std::type_index type) = 0;

public:
    template<typename T>
    inline T* query() { return static_cast<T*>(queryInterface(typeid(T))); }
};
```

**Purpose**:
- Runtime type discovery and interface querying
- Similar to COM's `QueryInterface` pattern
- Allows SDK to extend interfaces without breaking ABI compatibility
- Applications can probe for new features at runtime

**Usage**:
```cpp
SR::IDisplay* display = displayMgr->getPrimaryActiveSRDisplay();
// Query for potential future extended interface
SomeNewInterface* extended = display->query<SomeNewInterface>();
if (extended) {
    // New feature available
}
```

## Error Handling Philosophy

### Old Approach (Display/Screen classes)
- Assume display always exists
- Methods may throw or return invalid data
- Application crashes on missing display

### New Approach (IDisplayManager/IDisplay)
1. **Manager Level**: `TryGetDisplayManagerInstance()` returns `nullptr` if interface doesn't exist
2. **Display Level**: `getPrimaryActiveSRDisplay()` always returns a valid pointer
3. **Validity Check**: `isValid()` indicates whether display exists
4. **Safe Defaults**: All getters return sensible defaults when `isValid() == false`

This multi-layered approach ensures:
- No crashes from missing displays
- No crashes from older runtimes
- Graceful degradation at every level
- Applications can adapt behavior based on capabilities

## Migration Path

### From Screen → Display
```cpp
// Old (deprecated):
SR::Screen* screen = SR::Screen::create(context);
int width = screen->getResolutionWidth();

// New:
SR::Display* display = SR::Display::create(context);
int width = display->getResolutionWidth();
```

### From Display → IDisplayManager/IDisplay
```cpp
// Old:
SR::Display* display = SR::Display::create(context);
int width = display->getResolutionWidth();

// New (version-safe):
#define SRDISPLAY_LAZYBINDING
#include "sr/world/display/display.h"

SR::IDisplayManager* mgr = SR::TryGetDisplayManagerInstance(context);
if (mgr) {
    SR::IDisplay* display = mgr->getPrimaryActiveSRDisplay();
    if (display->isValid()) {
        int width = display->getResolutionWidth();
        float x, y, z;
        display->getDefaultViewingPosition(x, y, z);  // New capability!
    }
}

```

## Technical Details

### DLL Name Resolution Order

`TryGetDisplayManagerInstance()` searches for modules in this order:
1. `SimulatedRealityDisplays.lib` - Release x64
2. `SimulatedRealityDisplaysd.lib` - Debug x64
3. `SimulatedRealityDisplays32.lib` - Release x86
4. `SimulatedRealityDisplays32d.lib` - Debug x86

This ensures compatibility across build configurations.

### Platform Support

- **Windows Only**: Uses Win32 `GetModuleHandleA()` and `GetProcAddress()`
- Lazy binding only enabled with `SRDISPLAY_LAZYBINDING` and `WIN32` defined
- On other platforms, use `GetDisplayManagerInstance()` directly (will fail on old runtimes)

### Performance Characteristics

- **First call**: ~microseconds (DLL lookup + symbol resolution)
- **Subsequent calls**: ~nanoseconds (cached function pointer dereference)
- **Memory overhead**: Single function pointer per process (8 bytes on x64)

## Recommendations

### For New Applications
✅ Use `IDisplayManager` with lazy binding for maximum compatibility
✅ Always check `isValid()` before relying on display parameters
✅ Define `SRDISPLAY_LAZYBINDING` before including `display.h`

### For Existing Applications
⚠️ Migrate from `Screen` to `Display` immediately (Screen is deprecated)
⚠️ Consider migrating from `Display` to `IDisplayManager` for better error handling
⚠️ Use `TryGetDisplayManagerInstance()` if you need to support multiple runtime versions

### For SDK Developers
🔧 Extend interfaces through `IQueryInterface::query<T>()` pattern
🔧 Maintain backward compatibility via lazy binding patterns
🔧 Use `isValid()` pattern for optional hardware detection

## Summary

The new DisplayManager architecture in SDK 1.34.8-RC1 represents a significant improvement in version compatibility and error handling:

1. **Lazy Binding**: Optional runtime symbol resolution prevents load failures
2. **Graceful Degradation**: Multi-level validity checks instead of crashes
3. **Interface Querying**: Runtime capability detection via `IQueryInterface`
4. **Safe Defaults**: Methods return valid values even when display not found
5. **Future-Proof**: Extensible design allows new features without breaking changes

This design allows applications built with SDK 1.34.8-RC1 to run on older runtimes while taking advantage of new features when available.
