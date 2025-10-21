# LookAround Eyes Comparison App - Product Requirements Document

**Version:** 1.0
**Date:** 2025-01-15
**Author:** Development Team
**SDK Version:** Leia SR SDK 1.34.8.31132

---

## 1. Project Overview

### 1.1 Purpose
Create a Windows application that enables real-time comparison between two different eye position tracking methods provided by the Leia SR SDK:
1. **Weaver Method**: `weaver->getPredictedEyePositions()`
2. **EyePairListener Method**: Callback-based `SR::EyePairListener`

### 1.2 Goals
- Allow developers to visually compare the behavior and accuracy of both eye tracking methods
- Provide instant runtime switching between methods via hotkey
- Render a 3D scene with multiple objects at varying depths to highlight parallax differences
- Display real-time information about the active tracking method and performance metrics

### 1.3 Target Users
- Leia SDK developers evaluating eye tracking methods
- Application developers choosing between tracking approaches
- QA engineers testing eye tracking accuracy

---

## 2. Technical Stack

### 2.1 Graphics API
- **DirectX 11** - Immediate context API for simpler debugging and compatibility
- Uses `ID3D11Device`, `ID3D11DeviceContext`, and `ID3D11SwapChain`

### 2.2 SDK Dependencies
- Leia SR SDK 1.34.8.31132 (Win64 Release)
- Core components:
  - `SR::SRContext` - Connection to SR Platform Service
  - `SR::IDX11Weaver1` - Modern weaver interface
  - `SR::EyeTracker` - Eye tracking service
  - `SR::SystemSense` - System event monitoring
  - `SR::Display` - Display properties

### 2.3 Additional Libraries
- **DirectXMath** - Math library for transforms and vectors
- **STB Image** (optional) - For loading textures if needed
- **Windows API** - Window management, input handling

### 2.4 Development Environment
- Visual Studio 2019 or later
- Windows 10/11 SDK
- C++17 or later

---

## 3. Core Features

### 3.1 Dual Eye Tracking System

#### 3.1.1 Method 1: Weaver Predicted Positions
```cpp
// Modern API - automatic latency compensation
float leftEye[3], rightEye[3];
g_weaver->getPredictedEyePositions(leftEye, rightEye);
```

**Characteristics:**
- Automatic latency prediction based on `setLatencyInFrames()`
- Integrated with weaver lifecycle
- Minimal code required

#### 3.1.2 Method 2: EyePairListener Callback
```cpp
// Legacy pattern - raw tracking data with callbacks
class SREyePairListener : public SR::EyePairListener {
    vec3f left, right;
    virtual void accept(const SR_eyePair& eyePair) override {
        left = vec3f(eyePair.left.x, eyePair.left.y, eyePair.left.z);
        right = vec3f(eyePair.right.x, eyePair.right.y, eyePair.right.z);
    }
};
```

**Characteristics:**
- Direct access to raw eye tracking data
- Includes timestamp and frame ID
- Requires manual lifecycle management

### 3.2 Runtime Mode Switching

**Hotkey:** `Tab` key

**Behavior:**
- Instant switch between methods with no application restart
- Maintains both tracking systems active simultaneously
- Seamless transition without visual glitches
- Updates on-screen display to reflect current mode

**State Management:**
```cpp
enum class EyeTrackingMode {
    WeaverMethod,      // Using weaver->getPredictedEyePositions()
    ListenerMethod     // Using EyePairListener callbacks
};

EyeTrackingMode g_currentMode = EyeTrackingMode::WeaverMethod;
```

### 3.3 3D Scene Rendering

#### Scene Composition: Multiple Objects at Different Depths

**Near Plane Objects** (200mm from screen):
- **Red Cube** - Position: (0mm, 100mm, 200mm)
- Size: 40mm × 40mm × 40mm
- Purpose: Test close-range tracking accuracy

**Mid Plane Objects** (400mm from screen):
- **Green Sphere** - Position: (-80mm, 0mm, 400mm)
- **Blue Sphere** - Position: (80mm, 0mm, 400mm)
- Radius: 30mm each
- Purpose: Test mid-range parallax separation

**Far Plane Objects** (700mm from screen):
- **Yellow Cube** - Position: (0mm, -100mm, 700mm)
- Size: 60mm × 60mm × 60mm
- Purpose: Test long-range depth perception

**Background Grid** (1000mm from screen):
- Reference grid plane for depth context
- Subtle color to not distract from main objects

#### Rendering Strategy
- All objects rotate slowly to provide motion parallax cues
- Different rotation speeds for each depth layer
- Vertex colors (no textures) for simplicity
- Depth buffer enabled for proper occlusion

### 3.4 On-Screen Display (HUD)

**Position:** Top-left corner overlay

**Information Displayed:**

1. **Current Eye Tracking Mode**
   ```
   Eye Tracking: [WEAVER METHOD] / [LISTENER METHOD]
   ```
   - Green text for active mode
   - Large, readable font

2. **FPS Counter**
   ```
   FPS: 60.0
   ```
   - Updated every 250ms
   - White text

**Rendering:**
- Direct2D overlay OR DirectWrite text rendering
- Semi-transparent black background for readability
- Does not interfere with 3D scene

### 3.5 Keyboard Controls

| Key | Action |
|-----|--------|
| `Tab` | Toggle between Weaver Method and EyePairListener Method |
| `F11` | Toggle fullscreen/windowed mode |
| `Esc` | Exit application |
| `C` | Simulate context invalidation (for testing recovery) |

---

## 4. Architecture

### 4.1 Application Structure

```
LookAroundEyesApp/
├── src/
│   ├── main.cpp              // Entry point, window creation, message loop
│   ├── Renderer.cpp/.h       // DirectX 11 rendering system
│   ├── EyeTracker.cpp/.h     // Dual eye tracking management
│   ├── Scene.cpp/.h          // 3D scene objects and geometry
│   ├── HUD.cpp/.h            // On-screen display overlay
│   └── Math.h                // vec3f, vec4f, mat4f utilities
├── shaders/
│   ├── VertexShader.hlsl     // Basic vertex transformation
│   └── PixelShader.hlsl      // Solid color output
├── resources/
│   └── icon.ico              // Application icon
└── LookAroundEyesApp.vcxproj // Visual Studio project
```

### 4.2 Key Classes

#### 4.2.1 DualEyeTracker
```cpp
class DualEyeTracker {
public:
    enum class Mode { Weaver, Listener };

    DualEyeTracker(SR::SRContext* context, SR::IDX11Weaver1* weaver);
    ~DualEyeTracker();

    void setMode(Mode mode);
    Mode getMode() const;

    void getEyePositions(vec3f& leftEye, vec3f& rightEye);

private:
    Mode m_currentMode;
    SR::IDX11Weaver1* m_weaver;
    SR::EyeTracker* m_eyeTracker;
    SREyePairListener* m_listener;  // Always active for comparison
};
```

#### 4.2.2 Scene
```cpp
class Scene {
public:
    void initialize(ID3D11Device* device,
                    float screenWidthMM,
                    float screenHeightMM,
                    float virtualScreenDepthMM);

    void render(ID3D11DeviceContext* context,
                const vec3f& eyePosition,
                float elapsedTime);

private:
    struct SceneObject {
        vec3f position;
        vec3f color;
        float size;
        ObjectType type;  // Cube, Sphere
        float rotationSpeed;
        ID3D11Buffer* vertexBuffer;
        ID3D11Buffer* indexBuffer;
        uint32_t indexCount;
    };

    std::vector<SceneObject> m_objects;
    ID3D11Buffer* m_constantBuffer;
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;

    // Display properties for view-projection calculation
    float m_screenWidthMM;
    float m_screenHeightMM;
    float m_virtualScreenDepthMM;
};
```

#### 4.2.3 HUD
```cpp
class HUD {
public:
    void initialize(ID3D11Device* device);
    void render(ID3D11DeviceContext* context,
                DualEyeTracker::Mode mode,
                float fps);

private:
    ID2D1Factory* m_d2dFactory;
    ID2D1RenderTarget* m_d2dRenderTarget;
    IDWriteFactory* m_writeFactory;
    IDWriteTextFormat* m_textFormat;
    ID2D1SolidColorBrush* m_whiteBrush;
    ID2D1SolidColorBrush* m_greenBrush;
};
```

### 4.3 Rendering Pipeline

```
Frame Start
    ↓
Update Eye Tracker
    ↓
Get Eye Positions (via current mode)
    ↓
Clear Stereo View Texture (2× width)
    ↓
Render Left Eye View
    ├── Set viewport to left half
    ├── Calculate view-projection for left eye
    └── Render scene objects
    ↓
Render Right Eye View
    ├── Set viewport to right half
    ├── Calculate view-projection for right eye
    └── Render scene objects
    ↓
Set Backbuffer as Render Target
    ↓
Perform Weaving
    ↓
Render HUD Overlay
    ↓
Present
```

---

## 5. Implementation Details

### 5.1 Initialization Sequence

```cpp
// 1. Create SR Context
SR::SRContext* context = SR::SRContext::create();

// 2. Wait for display ready and get display properties
#define SRDISPLAY_LAZYBINDING
SR::IDisplay* display = SR::GetMainSRDisplay(*context);
if (!display || !display->isValid()) {
    // Fallback or error handling
}

// 3. Get recommended texture dimensions from IDisplay
int viewWidth = display->getRecommendedViewsTextureWidth();
int viewHeight = display->getRecommendedViewsTextureHeight();

// 4. Get physical screen dimensions (in millimeters)
float screenWidthMM = display->getPhysicalSizeWidth() * 10.0f;   // cm to mm
float screenHeightMM = display->getPhysicalSizeHeight() * 10.0f; // cm to mm

// 5. Get default viewing position to derive camera parameters
float defaultViewX, defaultViewY, defaultViewZ;
display->getDefaultViewingPosition(defaultViewX, defaultViewY, defaultViewZ);

// 6. Calculate camera FOV from viewing pyramid
// The physical screen and viewing distance form a pyramid
float viewingDistanceMM = defaultViewZ;  // Distance from eye to screen
float horizontalFOV = 2.0f * atan2f(screenWidthMM / 2.0f, viewingDistanceMM);
float verticalFOV = 2.0f * atan2f(screenHeightMM / 2.0f, viewingDistanceMM);

// 7. Store global scene parameters
g_ScreenWidthInMM = screenWidthMM;
g_ScreenHeightInMM = screenHeightMM;
g_VirtualScreenDepthMM = 400.0f;  // Set virtual screen at 400mm to focus mid-plane objects

// 8. Initialize DirectX 11
CreateDevice(&d3d11Device, &d3d11Context);
CreateSwapChain(hwnd, &swapChain);

// 9. Create stereo view texture (2× width for side-by-side)
CreateStereoTexture(viewWidth * 2, viewHeight, &viewTexture, &viewRTV, &viewSRV);

// 10. Create weaver
SR::CreateDX11Weaver(context, d3d11Context, hwnd, &weaver);
weaver->setInputViewTexture(viewSRV, viewWidth, viewHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
weaver->setLatencyInFrames(2);

// 11. Initialize dual eye tracker
DualEyeTracker eyeTracker(context, weaver);

// 12. Initialize scene and HUD with derived parameters
scene.initialize(d3d11Device, screenWidthMM, screenHeightMM, g_VirtualScreenDepthMM);
hud.initialize(d3d11Device);

// 13. Initialize context (AFTER weaver creation)
context->initialize();
```

### 5.2 Main Render Loop

```cpp
void Render()
{
    // Update time
    float currentTime = GetTime();
    float deltaTime = currentTime - lastFrameTime;

    // Get eye positions using current tracking mode
    vec3f leftEye, rightEye;
    eyeTracker.getEyePositions(leftEye, rightEye);

    // Clear stereo view texture
    const float clearColor[4] = {0.1f, 0.1f, 0.2f, 1.0f};
    d3d11Context->ClearRenderTargetView(viewRTV, clearColor);
    d3d11Context->ClearDepthStencilView(viewDepthDSV,
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Set stereo texture as render target
    d3d11Context->OMSetRenderTargets(1, &viewRTV, viewDepthDSV);

    // Render both eyes
    for (int i = 0; i < 2; i++)
    {
        // Set viewport for this eye
        D3D11_VIEWPORT viewport;
        viewport.TopLeftX = (FLOAT)(i * viewWidth);
        viewport.TopLeftY = 0.0f;
        viewport.Width = (FLOAT)viewWidth;
        viewport.Height = (FLOAT)viewHeight;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        d3d11Context->RSSetViewports(1, &viewport);

        // Render scene for this eye using derived FOV and virtual screen depth
        vec3f eyePos = (i == 0) ? leftEye : rightEye;
        scene.render(d3d11Context, eyePos, currentTime);
    }

    // Note: Scene internally uses:
    // - screenWidthMM, screenHeightMM from IDisplay->getPhysicalSize*()
    // - virtualScreenDepthMM = 400mm (mid-plane focus)
    // - FOV derived from display pyramid (screen size / viewing distance)

    // Set backbuffer as render target
    d3d11Context->OMSetRenderTargets(1, &backbufferRTV, nullptr);

    // Reset viewport to full window
    D3D11_VIEWPORT fullViewport;
    fullViewport.TopLeftX = 0.0f;
    fullViewport.TopLeftY = 0.0f;
    fullViewport.Width = (FLOAT)windowWidth;
    fullViewport.Height = (FLOAT)windowHeight;
    fullViewport.MinDepth = 0.0f;
    fullViewport.MaxDepth = 1.0f;
    d3d11Context->RSSetViewports(1, &fullViewport);

    // Perform weaving
    weaver->weave();

    // Render HUD overlay
    hud.render(d3d11Context, eyeTracker.getMode(), currentFPS);

    // Present
    swapChain->Present(1, 0);  // vsync

    // Update FPS counter
    UpdateFPS(deltaTime);
}
```

### 5.3 Input Handling

```cpp
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_TAB:
        {
            // Toggle eye tracking mode
            auto currentMode = eyeTracker.getMode();
            auto newMode = (currentMode == DualEyeTracker::Mode::Weaver)
                ? DualEyeTracker::Mode::Listener
                : DualEyeTracker::Mode::Weaver;
            eyeTracker.setMode(newMode);

            // Optional: Play sound or flash indicator
            return 0;
        }

        case VK_F11:
            ToggleFullscreen(hWnd);
            return 0;

        case VK_ESCAPE:
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            return 0;

        case 'C':
        case 'c':
            // Simulate context invalidation for testing
            SimulateContextInvalidation();
            return 0;
        }
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
```

### 5.4 Scene Object Creation

```cpp
void Scene::createNearPlaneObjects()
{
    // Red cube at 200mm depth
    SceneObject redCube;
    redCube.position = vec3f(0.0f, 100.0f, 200.0f);
    redCube.color = vec3f(1.0f, 0.2f, 0.2f);  // Red
    redCube.size = 40.0f;
    redCube.type = ObjectType::Cube;
    redCube.rotationSpeed = 0.5f;  // radians/second

    CreateCubeGeometry(redCube);
    m_objects.push_back(redCube);
}

void Scene::createMidPlaneObjects()
{
    // Green sphere (left)
    SceneObject greenSphere;
    greenSphere.position = vec3f(-80.0f, 0.0f, 400.0f);
    greenSphere.color = vec3f(0.2f, 1.0f, 0.2f);  // Green
    greenSphere.size = 30.0f;
    greenSphere.type = ObjectType::Sphere;
    greenSphere.rotationSpeed = 0.3f;

    CreateSphereGeometry(greenSphere);
    m_objects.push_back(greenSphere);

    // Blue sphere (right)
    SceneObject blueSphere;
    blueSphere.position = vec3f(80.0f, 0.0f, 400.0f);
    blueSphere.color = vec3f(0.2f, 0.2f, 1.0f);  // Blue
    blueSphere.size = 30.0f;
    blueSphere.type = ObjectType::Sphere;
    blueSphere.rotationSpeed = 0.3f;

    CreateSphereGeometry(blueSphere);
    m_objects.push_back(blueSphere);
}

void Scene::createFarPlaneObjects()
{
    // Yellow cube at 700mm depth
    SceneObject yellowCube;
    yellowCube.position = vec3f(0.0f, -100.0f, 700.0f);
    yellowCube.color = vec3f(1.0f, 1.0f, 0.2f);  // Yellow
    yellowCube.size = 60.0f;
    yellowCube.type = ObjectType::Cube;
    yellowCube.rotationSpeed = 0.2f;

    CreateCubeGeometry(yellowCube);
    m_objects.push_back(yellowCube);
}
```

---

## 6. Camera Configuration from IDisplay

### 6.1 Getting Display Parameters

The application must query the IDisplay interface to properly configure the camera and rendering:

```cpp
// Get the modern IDisplay interface
#define SRDISPLAY_LAZYBINDING
SR::IDisplay* display = SR::GetMainSRDisplay(*context);

// Critical parameters for camera setup:

// 1. View texture dimensions (optimized for the display)
int viewWidth = display->getRecommendedViewsTextureWidth();
int viewHeight = display->getRecommendedViewsTextureHeight();

// 2. Physical screen dimensions (for FOV calculation)
float screenWidthCM = display->getPhysicalSizeWidth();    // centimeters
float screenHeightCM = display->getPhysicalSizeHeight();  // centimeters
float screenWidthMM = screenWidthCM * 10.0f;              // convert to mm
float screenHeightMM = screenHeightCM * 10.0f;            // convert to mm

// 3. Default viewing position (defines the viewing pyramid)
float viewPosX, viewPosY, viewPosZ;
display->getDefaultViewingPosition(viewPosX, viewPosY, viewPosZ);
// Typically: viewPosX = 0, viewPosY = 0, viewPosZ = 600-700mm
```

### 6.2 Viewing Pyramid and FOV Derivation

The physical screen and default viewing position form a **viewing pyramid**:

```
                    Eye at (0, 0, viewPosZ)
                            *  ← Apex (default viewing position)
                           /|\
                          / | \
                         /  |  \
                        /   |   \
                       /    |    \  FOV angle derived from geometry
                      /     |     \
                     /      |Z     \
                    /       |       \
                   /        |        \
                  *---------+---------*
               Screen at Z=0 (physical dimensions)
```

**Mathematical Derivation:**

```cpp
// The half-angle of the FOV is given by:
// tan(halfFOV) = (half_screen_dimension) / viewing_distance

// Therefore, full FOV is:
float horizontalFOV = 2.0f * atan2f(screenWidthMM / 2.0f, viewPosZ);
float verticalFOV = 2.0f * atan2f(screenHeightMM / 2.0f, viewPosZ);

// Example with typical Leia display:
// Screen: 600mm × 340mm
// Viewing distance: 650mm
// Horizontal FOV = 2 * atan(300/650) = 48.4 degrees
// Vertical FOV = 2 * atan(170/650) = 29.3 degrees
```

### 6.3 Virtual Screen Positioning

**Key Concept**: The virtual screen doesn't have to be at Z=0 (physical display location).

For this application, we position the virtual screen at **Z=400mm** to:
- Match the depth of mid-plane objects (green/blue spheres)
- Create balanced parallax distribution
- Optimize the 3D "sweet spot" for comparison

```cpp
const float VIRTUAL_SCREEN_DEPTH_MM = 400.0f;

// When calculating view-projection matrix:
// - Screen plane corners are at (x, y, 400) instead of (x, y, 0)
// - Objects at 400mm appear exactly at the virtual screen surface
// - Objects < 400mm appear to pop out (positive parallax)
// - Objects > 400mm appear to recede (negative parallax)
```

### 6.4 Scene Depth Distribution

With virtual screen at 400mm, our object distribution becomes:

| Object | Z Position | Distance from Virtual Screen | Visual Effect |
|--------|-----------|------------------------------|---------------|
| Red Cube | 200mm | -200mm (closer) | Pops out strongly |
| Green Sphere | 400mm | 0mm (at screen) | **At virtual screen surface** |
| Blue Sphere | 400mm | 0mm (at screen) | **At virtual screen surface** |
| Yellow Cube | 700mm | +300mm (farther) | Recedes moderately |
| Grid | 1000mm | +600mm (farther) | Deep background |

This ensures maximum visibility of parallax differences when comparing eye tracking methods.

---

## 7. Shader Implementation

### 7.1 Vertex Shader
```hlsl
// VertexShader.hlsl
cbuffer Constants : register(b0)
{
    matrix viewProjection;
    matrix model;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float3 color    : COLOR;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 color    : COLOR;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 worldPos = mul(model, float4(input.position, 1.0));
    output.position = mul(viewProjection, worldPos);
    output.color = input.color;

    return output;
}
```

### 7.2 Pixel Shader
```hlsl
// PixelShader.hlsl
struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 color    : COLOR;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    return float4(input.color, 1.0);
}
```

---

## 8. Testing Criteria

### 8.1 Visual Comparison Tests

When switching between modes, observe:

1. **Depth Perception Consistency**
   - Do objects maintain proper depth separation?
   - Does the 3D effect remain stable?

2. **Parallax Accuracy**
   - Near plane (red cube): Should have strongest parallax
   - Mid plane (green/blue spheres): Moderate parallax
   - Far plane (yellow cube): Subtle parallax

3. **Tracking Smoothness**
   - Are eye position updates smooth or jittery?
   - Does head movement tracking feel responsive?

4. **Latency Differences**
   - Does one method feel more responsive?
   - Is there noticeable lag in viewport adjustment?

5. **3D Sweet Spot**
   - Does the optimal viewing position differ between methods?
   - Is the 3D effect equally stable when moving?

### 8.2 Performance Metrics

Monitor the FPS counter:
- **Target**: 60 FPS (vsync)
- Compare if switching modes affects performance
- Note any frame drops during mode transitions

### 8.3 Edge Cases

Test scenarios:
- **Rapid mode switching**: Press Tab repeatedly
- **User movement**: Move head while switching modes
- **Occlusion**: Check depth ordering at object intersections
- **Extended use**: Run for 5+ minutes in each mode

---

## 9. Build Configuration

### 9.1 Project Properties

**Platform:** x64 (Win64)

**C/C++ → General → Additional Include Directories:**
```
$(SolutionDir)simulatedreality-SDK-1.34.8.31132-win64-Release\include
$(SolutionDir)src
```

**C/C++ → Language:**
- C++ Language Standard: C++17 or later

**Linker → General → Additional Library Directories:**
```
$(SolutionDir)simulatedreality-SDK-1.34.8.31132-win64-Release\lib\x64
```

**Linker → Input → Additional Dependencies:**
```
d3d11.lib
d3dcompiler.lib
dxgi.lib
shcore.lib
d2d1.lib
dwrite.lib
SimulatedRealityCore.lib
SimulatedRealityFaceTrackers.lib
SimulatedRealitySense.lib
SimulatedRealityDisplays.lib
SimulatedRealityWeaving.lib
```

**Build Events → Post-Build:**
```batch
xcopy /Y /D "$(SolutionDir)simulatedreality-SDK-1.34.8.31132-win64-Release\bin\x64\*.dll" "$(OutDir)"
```

### 9.2 Runtime Requirements

**DLL Dependencies:**
Must be in same directory as executable:
- `SimulatedRealityCore.dll`
- `SimulatedRealityFaceTrackers.dll`
- `SimulatedRealitySense.dll`
- `SimulatedRealityDisplays.dll`
- `SimulatedRealityWeaving.dll`

**External Services:**
- Leia SR Platform Service must be running
- Leia display must be connected and configured

---

## 10. Success Metrics

### 10.1 Functional Requirements ✓

- [ ] Application launches successfully on Leia display
- [ ] Scene renders correctly in 3D with both eyes
- [ ] Tab key toggles between eye tracking methods
- [ ] HUD displays current mode accurately
- [ ] FPS counter updates in real-time
- [ ] All five scene objects render at correct depths
- [ ] F11 toggles fullscreen mode
- [ ] Esc key exits application cleanly

### 10.2 Comparison Objectives ✓

- [ ] Clear visual difference observable when switching modes (if any)
- [ ] No crashes or visual artifacts during mode switching
- [ ] Both modes provide acceptable 3D viewing experience
- [ ] Performance remains consistent (60 FPS) in both modes
- [ ] Context invalidation recovery works (C key test)

### 10.3 Code Quality ✓

- [ ] No memory leaks (verified with Visual Studio diagnostics)
- [ ] Proper cleanup of DirectX and SR resources
- [ ] Thread-safe access to eye tracking data
- [ ] Follows Leia SDK integration best practices

---

## 11. Future Enhancements (Optional)

### 11.1 Additional Features

1. **Eye Position Display**
   - Show actual XYZ coordinates of both eyes
   - Display delta between the two methods

2. **Recording Mode**
   - Log eye positions over time to CSV file
   - Compare methods quantitatively

3. **Custom Latency Control**
   - UI slider to adjust `setLatencyInFrames()` on the fly
   - Compare effect on weaver method prediction

4. **More Complex Scenes**
   - Textured objects
   - Dynamic lighting
   - Particle effects

### 11.2 Cross-Platform

- OpenGL version for non-Windows platforms
- Vulkan version for modern low-level API comparison

---

## 12. Known Limitations

1. **DirectX 11 Only**: Not cross-platform (Windows-specific)
2. **Single User**: Eye tracking optimized for one viewer
3. **Fixed Scene**: Objects don't respond to user input (intentional for controlled comparison)
4. **No Late Latching UI**: Late latching enabled by default, no runtime toggle

---

## 13. References

- Leia SR SDK Documentation (`SR_SDK_INTEGRATION_GUIDE.md`)
- DirectX 11 Programming Guide (Microsoft Docs)
- `LookAroundEyes.md` - Summary of eye tracking methods
- SDK Examples:
  - `examples/directx11_weaving/` - Reference implementation
  - `examples/opengl_weaving/` - Alternative graphics API example

---

## 14. Appendix: Math Utilities

### 14.1 View-Projection Matrix Calculation

Using generalized perspective projection (Kooima 2009) with virtual screen at 400mm:

```cpp
mat4f CalculateViewProjectionMatrix(const vec3f& eyePosition,
                                     float screenWidthMM,
                                     float screenHeightMM,
                                     float virtualScreenDepthMM)
{
    const float znear = 0.1f;
    const float zfar = 10000.0f;

    // CRITICAL: Virtual screen is positioned at 400mm depth (not at 0mm)
    // This places the screen plane in the middle of our scene, keeping
    // the mid-plane objects (green/blue spheres at 400mm) perfectly in focus

    // Screen corners in world space (at virtual screen depth)
    vec3f pa(-screenWidthMM / 2.0f,  screenHeightMM / 2.0f, virtualScreenDepthMM);  // Top-left
    vec3f pb( screenWidthMM / 2.0f,  screenHeightMM / 2.0f, virtualScreenDepthMM);  // Top-right
    vec3f pc(-screenWidthMM / 2.0f, -screenHeightMM / 2.0f, virtualScreenDepthMM);  // Bottom-left

    // Screen basis vectors
    vec3f vr(1.0f, 0.0f, 0.0f);  // Right
    vec3f vu(0.0f, 1.0f, 0.0f);  // Up
    vec3f vn(0.0f, 0.0f, 1.0f);  // Normal (toward viewer)

    // Vectors from eye to screen corners
    vec3f va = pa - eyePosition;
    vec3f vb = pb - eyePosition;
    vec3f vc = pc - eyePosition;

    // Distance from eye to screen plane
    float distance = -vec3f::dot(va, vn);

    // Frustum extents at near plane
    float l = vec3f::dot(vr, va) * znear / distance;
    float r = vec3f::dot(vr, vb) * znear / distance;
    float b = vec3f::dot(vu, vc) * znear / distance;
    float t = vec3f::dot(vu, va) * znear / distance;

    // Asymmetric frustum matrix
    mat4f frustum = mat4f::perspective(l, r, b, t, znear, zfar);

    // Translation to move eye to origin
    mat4f translate = mat4f::translation(-eyePosition);

    return frustum * translate;
}
```

### 14.2 Camera FOV Calculation from Display Properties

The camera's field of view is derived from the physical screen dimensions and default viewing position:

```cpp
// Derive FOV from viewing pyramid
// The pyramid is formed by:
//   - Base: Physical screen dimensions (width × height in mm)
//   - Apex: Default viewing position (at distance Z from screen)
//
//     Viewing Position (eye)
//            *
//           /|\
//          / | \
//         /  |  \
//        /   |Z  \
//       /    |    \
//      /     |     \
//     *------+------*
//    Physical Screen
//
// FOV = 2 * atan(half_screen_dimension / viewing_distance)

float screenWidthMM = display->getPhysicalSizeWidth() * 10.0f;   // cm to mm
float screenHeightMM = display->getPhysicalSizeHeight() * 10.0f; // cm to mm

float defaultViewX, defaultViewY, defaultViewZ;
display->getDefaultViewingPosition(defaultViewX, defaultViewY, defaultViewZ);

float viewingDistanceMM = defaultViewZ;  // Z distance to screen

// Calculate horizontal and vertical FOV (in radians)
float horizontalFOV = 2.0f * atan2f(screenWidthMM / 2.0f, viewingDistanceMM);
float verticalFOV = 2.0f * atan2f(screenHeightMM / 2.0f, viewingDistanceMM);

// Convert to degrees if needed
float horizontalFOVDegrees = horizontalFOV * (180.0f / M_PI);
float verticalFOVDegrees = verticalFOV * (180.0f / M_PI);

// Example typical values:
// - Screen: 600mm × 340mm
// - Viewing distance: 650mm
// - Horizontal FOV: ~48 degrees
// - Vertical FOV: ~28 degrees
```

### 14.3 Virtual Screen Depth Rationale

**Why 400mm?**

Our scene has objects at three depth planes:
- Near: 200mm (red cube)
- **Mid: 400mm (green/blue spheres)** ← Virtual screen placed here
- Far: 700mm (yellow cube)

By positioning the virtual screen at 400mm:

1. **Optimal Focus**: Mid-plane objects appear exactly at the screen surface
2. **Balanced Parallax**:
   - Near objects (200mm) pop out 200mm in front of screen
   - Far objects (700mm) recede 300mm behind screen
3. **Depth Distribution**: Better utilization of positive and negative parallax
4. **User Comfort**: Reduces eye strain by centering the "sweet spot"

**Object Depth Relative to Virtual Screen:**

```
User's Eyes          Virtual Screen         Objects
     |                     |                   |
     |<---- eyeZ ---->|    |    |<-- 200mm -->| Red Cube (pops out)
     |                     |====|              | Green/Blue (at screen)
     |                     |    |<-- 300mm -->| Yellow (recedes)
     |                     |                   |
   600mm              400mm                 200-700mm
```

This configuration ensures the comparison between eye tracking methods is most visible at the mid-plane objects where parallax is most critical.

---

**End of PRD**
