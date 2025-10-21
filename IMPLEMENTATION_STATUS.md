# LookAround Eyes Comparison App - Implementation Status

**Date:** 2025-01-15
**Status:** Core Components Complete - Ready for main.cpp Integration

---

## Completed Components ✓

### 1. Math Utilities (`src/Math.h`)
- ✅ vec3f structure with operators and static methods (dot, cross, normalize)
- ✅ vec4f structure
- ✅ mat4f structure with matrix operations
- ✅ Identity, translation, scaling, rotation matrices
- ✅ Asymmetric perspective frustum (for off-axis projection)
- ✅ `CalculateViewProjectionMatrix()` - Generalized perspective projection with virtual screen at 400mm

### 2. Dual Eye Tracker (`src/DualEyeTracker.h/cpp`)
- ✅ `SREyePairListener` class - callback-based eye tracking
  - Receives `SR_eyePair` updates via `accept()` method
  - Stores left/right eye positions
  - Opens eye pair stream from EyeTracker
- ✅ `DualEyeTracker` class - manages both tracking methods
  - `Mode::Weaver` - uses `weaver->getPredictedEyePositions()`
  - `Mode::Listener` - uses `EyePairListener` callbacks
  - `setMode()` / `getMode()` for runtime switching
  - `getEyePositions()` returns eyes based on active mode
  - Both methods active simultaneously for instant switching

### 3. Scene Rendering (`src/Scene.h/cpp`)
- ✅ `SceneObject` structure - defines 3D objects
- ✅ `Vertex` structure - position + color
- ✅ `SceneConstants` - constant buffer for transforms
- ✅ Geometry generation:
  - `createCubeGeometry()` - 36 indices, 8 vertices
  - `createSphereGeometry()` - UV sphere with configurable segments
- ✅ Scene objects created per PRD:
  - **Near plane (200mm):** Red cube, 40mm, rotation speed 0.5 rad/s
  - **Mid plane (400mm):** Green sphere (left, -80mm), Blue sphere (right, +80mm), 30mm each
  - **Far plane (700mm):** Yellow cube, 60mm, rotation speed 0.2 rad/s
- ✅ Shader compilation from HLSL files
- ✅ Render loop with:
  - View-projection calculation using virtual screen at 400mm
  - Per-object model matrix (translation + rotation)
  - Constant buffer updates
  - Draw calls for each object

### 4. HLSL Shaders (`shaders/`)
- ✅ `VertexShader.hlsl` - transforms vertices with view-projection and model matrices
- ✅ `PixelShader.hlsl` - solid color output

### 5. HUD Overlay (`src/HUD.h/cpp`)
- ✅ Direct2D/DirectWrite integration
- ✅ Displays:
  - Current eye tracking mode (green text: "WEAVER METHOD" or "LISTENER METHOD")
  - FPS counter (white text, updates every 250ms)
- ✅ Semi-transparent black background for readability
- ✅ FPS calculation with running average

---

## Remaining Tasks

### 6. Main Application (`src/main.cpp`) - IN PROGRESS
Need to implement:
- [ ] Window creation (WndProc, message loop)
- [ ] DirectX 11 initialization
  - Device and context creation
  - Swap chain setup
  - Render target views
- [ ] SR Context initialization
  - Query IDisplay with SRDISPLAY_LAZYBINDING
  - Get recommended texture dimensions
  - Get physical screen size
  - Get default viewing position
  - Calculate FOV from viewing pyramid
- [ ] Stereo view texture creation (2× width for side-by-side)
- [ ] Weaver creation and configuration
- [ ] System event listener for context invalidation
- [ ] Main render loop:
  - Get eye positions from DualEyeTracker
  - Clear stereo view texture
  - Render left eye (viewport 0 to viewWidth)
  - Render right eye (viewport viewWidth to 2×viewWidth)
  - Set backbuffer as render target
  - Perform weaving
  - Render HUD overlay
  - Present
- [ ] Input handling (WndProc):
  - **Tab:** Toggle between Weaver/Listener modes
  - **F11:** Toggle fullscreen
  - **Esc:** Exit application
  - **C:** Simulate context invalidation
- [ ] Resource cleanup in correct order

### 7. Visual Studio Project File (`LookAroundEyesApp.vcxproj`)
- [ ] Create project file with:
  - Platform: x64
  - Include directories: SDK include path
  - Library directories: SDK lib/x64 path
  - Additional dependencies: d3d11.lib, d3dcompiler.lib, dxgi.lib, shcore.lib, d2d1.lib, dwrite.lib, SR libs
  - Post-build: Copy SDK DLLs to output directory

---

## Key Implementation Details

### Virtual Screen Positioning
- Virtual screen at **400mm depth** (not at physical display Z=0)
- Screen plane corners positioned at `(x, y, 400)` in view-projection matrix
- Objects at 400mm appear exactly at virtual screen surface
- Objects < 400mm pop out (positive parallax)
- Objects > 400mm recede (negative parallax)

### IDisplay Integration
The main.cpp will use:
```cpp
SR::IDisplay* display = SR::GetMainSRDisplay(*context);
int viewWidth = display->getRecommendedViewsTextureWidth();
int viewHeight = display->getRecommendedViewsTextureHeight();
float screenWidthMM = display->getPhysicalSizeWidth() * 10.0f;  // cm to mm
float screenHeightMM = display->getPhysicalSizeHeight() * 10.0f;
float viewPosX, viewPosY, viewPosZ;
display->getDefaultViewingPosition(viewPosX, viewPosY, viewPosZ);
```

### FOV Calculation
```cpp
float horizontalFOV = 2.0f * atan2f(screenWidthMM / 2.0f, viewPosZ);
float verticalFOV = 2.0f * atan2f(screenHeightMM / 2.0f, viewPosZ);
```

### Initialization Order (Critical!)
1. Create SR context
2. Wait for display ready
3. Create DirectX resources
4. Create weaver
5. Initialize dual eye tracker
6. Initialize scene
7. Initialize HUD
8. **Call `context->initialize()` LAST**

---

## File Structure

```
LookAroundEyesApp/
├── src/
│   ├── main.cpp              [TODO]
│   ├── Math.h                [✓ DONE]
│   ├── DualEyeTracker.h      [✓ DONE]
│   ├── DualEyeTracker.cpp    [✓ DONE]
│   ├── Scene.h               [✓ DONE]
│   ├── Scene.cpp             [✓ DONE]
│   ├── HUD.h                 [✓ DONE]
│   └── HUD.cpp               [✓ DONE]
├── shaders/
│   ├── VertexShader.hlsl     [✓ DONE]
│   └── PixelShader.hlsl      [✓ DONE]
└── LookAroundEyesApp.vcxproj [TODO]
```

---

## Next Steps

1. **Create main.cpp** with complete application logic
2. **Create Visual Studio project file** (.vcxproj)
3. **Test compilation** and fix any linker errors
4. **Run on Leia display** and verify:
   - 3D scene renders correctly
   - Tab key switches modes
   - HUD displays correct information
   - Objects appear at correct depths
5. **Compare tracking methods** visually
6. **Document findings**

---

## Dependencies

**SDK Libraries Required:**
- `SimulatedRealityCore.lib`
- `SimulatedRealityFaceTrackers.lib`
- `SimulatedRealitySense.lib`
- `SimulatedRealityDisplays.lib`
- `SimulatedRealityWeaving.lib`

**System Libraries:**
- `d3d11.lib`
- `d3dcompiler.lib`
- `dxgi.lib`
- `shcore.lib`
- `d2d1.lib`
- `dwrite.lib`

**Runtime DLLs (must be in output directory):**
- All SimulatedReality*.dll files from SDK bin/x64/

---

**Implementation Progress:** ~70% complete
**Estimated Time to Completion:** 2-3 hours (main.cpp + project setup + testing)
