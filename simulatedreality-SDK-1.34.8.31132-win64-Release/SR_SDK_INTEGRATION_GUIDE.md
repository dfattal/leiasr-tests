# Simulated Reality SDK Integration Guide

**SDK Version:** 1.34.8.31132
**Copyright:** © 2025 Leia, Inc.

---

## Table of Contents

1. [Overview](#overview)
2. [Core Components](#core-components)
3. [Display Interfaces](#display-interfaces)
4. [Initialization Workflow](#initialization-workflow)
5. [Rendering Loop](#rendering-loop)
6. [Side-by-Side Image Loading](#side-by-side-image-loading)
7. [Context Lifecycle Management](#context-lifecycle-management)
8. [DirectX 12 Differences](#directx-12-differences)
9. [Key Concepts](#key-concepts)
10. [Best Practices](#best-practices)

---

## Overview

The Simulated Reality (SR) SDK enables integration with Leia autostereoscopic displays, providing glasses-free 3D rendering capabilities. The SDK supports DirectX 9/10/11/12, OpenGL, and Vulkan graphics APIs.

### Key Features

- **Autostereoscopic Rendering**: Display 3D content without special glasses
- **Eye Tracking**: Real-time user eye position tracking (60+ Hz)
- **Weaving**: Automatic lens correction and multi-view generation
- **Late Latching**: Reduced latency through frame-in-flight eye position updates
- **Multi-API Support**: Works with all major graphics APIs

---

## Core Components

### SR Context (`SR::SRContext`)

Central connection manager to SR Platform Service.

**Responsibilities:**
- Manages connection to SR service
- Provides display properties
- Handles eye tracking
- Manages system events

**Creation:**
```cpp
SR::SRContext* context = SR::SRContext::create();
```

**Network Modes:**
- `NonBlockingClientMode` (default) - Try once, throw exception if unavailable
- `ClientMode` - Keep retrying until service available
- `StandaloneMode` - Client mode if server reachable, server mode otherwise
- `ServerMode` - Send data to client
- `EdgeMode` - Identical to ServerMode

---

### Weaver

Performs "weaving" - converts side-by-side stereo texture to autostereoscopic output.

**Modern API (SDK 1.34.8+):**
- `SR::IDX11Weaver1` - DirectX 11 interface
- `SR::IDX12Weaver1` - DirectX 12 interface
- `SR::IGLWeaver1` - OpenGL interface

**Legacy API (Deprecated):**
- `SR::PredictingDX11Weaver`
- `SR::PredictingDX12Weaver`
- `SR::DX11Weaver` / `SR::DX12Weaver`

**Responsibilities:**
- Lens correction
- Multi-view generation
- Crosstalk reduction
- Eye position prediction
- Late latching (optional)

---

### System Events (`SR::SystemEventListener`)

Monitors SR system status and lifecycle.

**Event Types:**
- `ContextInvalid` - Service crashed, need to recreate context
- `DeviceConnectedAndReady` / `DeviceDisconnected`
- `LensOn` / `LensOff` - Switchable lens state
- `UserFound` / `UserLost` - Eye tracking status
- `SRUnavailable` / `SRRestored`
- `DisplayNotConnected` / `DisplayNotConnectedResolved`
- `NonNativeResolution` / `NonNativeResolutionResolved`

---

## Display Interfaces

### Modern IDisplay Interface (SDK 1.34.8+)

**Preferred interface** with enhanced features and thread safety.

```cpp
class IDisplay : public SR::IQueryInterface
{
public:
    // Validation
    virtual bool isValid() const = 0;
    virtual uint64_t identifier() const = 0;
    virtual void refresh() = 0;

    // Resolution (what OS reports)
    virtual int getResolutionHeight() const = 0;
    virtual int getResolutionWidth() const = 0;

    // Physical panel resolution
    virtual int getPhysicalResolutionHeight() const = 0;
    virtual int getPhysicalResolutionWidth() const = 0;

    // Physical dimensions (centimeters)
    virtual float getPhysicalSizeHeight() const = 0;
    virtual float getPhysicalSizeWidth() const = 0;
    virtual float getDotPitch() const = 0;

    // Display location in Windows config
    virtual SR_recti getLocation() const = 0;

    // Recommended texture dimensions
    virtual int getRecommendedViewsTextureWidth() const = 0;
    virtual int getRecommendedViewsTextureHeight() const = 0;

    // Default viewing position (NEW!)
    virtual void getDefaultViewingPosition(float& x_mm, float& y_mm, float& z_mm) const = 0;
};
```

**Access:**
```cpp
SR::IDisplay* display = SR::GetMainSRDisplay(context);
if (display && display->isValid()) {
    // Use display
}
```

**Backward-Compatible Access (with lazy binding):**
```cpp
#define SRDISPLAY_LAZYBINDING
#include "sr/world/display/display.h"

SR::IDisplay* display = SR::TryGetMainSRDisplay(context);
if (display) {
    // Runtime supports IDisplay (SDK 1.34.8+)
    float x_mm, y_mm, z_mm;
    display->getDefaultViewingPosition(x_mm, y_mm, z_mm);
} else {
    // Fall back to legacy Display class
    SR::Display* legacyDisplay = SR::Display::create(context);
}
```

### Legacy Display Class (Pre-1.34.8)

```cpp
class Display : public WorldObject
{
public:
    static Display* create(SRContext& context);

    virtual const int getResolutionHeight() = 0;
    virtual const int getResolutionWidth() = 0;
    virtual const int getPhysicalResolutionHeight() = 0;
    virtual const int getPhysicalResolutionWidth() = 0;
    virtual const float getPhysicalSizeHeight() = 0;
    virtual const float getPhysicalSizeWidth() = 0;
    virtual const float getDotPitch() = 0;
    virtual SR_recti getLocation() = 0;
    virtual int getRecommendedViewsTextureWidth() = 0;
    virtual int getRecommendedViewsTextureHeight() = 0;
};
```

**Key Differences:**
- ❌ No `isValid()` method
- ❌ No `refresh()` method
- ❌ No `getDefaultViewingPosition()`
- ❌ Methods are non-const (not thread-safe)

---

## Initialization Workflow

### DirectX 11 Example

```cpp
// Step 1: Create SR Context with retry logic
SR::SRContext* g_srContext = nullptr;
const double startTime = (double)GetTickCount64() / 1000.0;
const double maxWaitTime = 10.0; // seconds

while (g_srContext == nullptr)
{
    try {
        g_srContext = SR::SRContext::create();
        break;
    }
    catch (SR::ServerNotAvailableException& e) {
        // SR service not available, retry
        Sleep(100);
    }

    double curTime = (double)GetTickCount64() / 1000.0;
    if ((curTime - startTime) > maxWaitTime)
        break; // Timeout
}

if (g_srContext == nullptr) {
    // Failed to create context
    return ERROR_NO_SR_SERVICE;
}

// Step 2: Wait for display to be ready
bool displayReady = false;
while (g_srContext && !displayReady)
{
    SR::Display* display = SR::Display::create(*g_srContext);
    if (display != nullptr) {
        SR_recti location = display->getLocation();
        int64_t width = location.right - location.left;
        int64_t height = location.bottom - location.top;
        if (width != 0 && height != 0) {
            displayReady = true;
            break;
        }
    }

    Sleep(100);

    double curTime = (double)GetTickCount64() / 1000.0;
    if ((curTime - startTime) > maxWaitTime)
        break; // Timeout
}

// Step 3: Get recommended view texture dimensions
SR::Display* display = SR::Display::create(*g_srContext);
int viewWidth = display->getRecommendedViewsTextureWidth();   // Single eye width
int viewHeight = display->getRecommendedViewsTextureHeight();

// Step 4: Create stereo view texture (2× width for side-by-side)
D3D11_TEXTURE2D_DESC textureDesc = {};
textureDesc.Width            = viewWidth * 2;  // Left + Right eyes
textureDesc.Height           = viewHeight;
textureDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
textureDesc.MipLevels        = 1;
textureDesc.ArraySize        = 1;
textureDesc.SampleDesc.Count = 1;
textureDesc.Usage            = D3D11_USAGE_DEFAULT;
textureDesc.BindFlags        = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

ID3D11Texture2D* viewTexture = nullptr;
g_device->CreateTexture2D(&textureDesc, nullptr, &viewTexture);

ID3D11ShaderResourceView* viewTextureSRV = nullptr;
g_device->CreateShaderResourceView(viewTexture, nullptr, &viewTextureSRV);

ID3D11RenderTargetView* viewTextureRTV = nullptr;
g_device->CreateRenderTargetView(viewTexture, nullptr, &viewTextureRTV);

// Step 5: Create DirectX 11 weaver (MODERN API)
SR::IDX11Weaver1* weaver = nullptr;
WeaverErrorCode result = SR::CreateDX11Weaver(
    g_srContext,           // SR context
    d3d11Context,          // ID3D11DeviceContext*
    hWnd,                  // Window handle
    &weaver
);

if (result != WeaverErrorCode::WeaverSuccess) {
    // Failed to create weaver
    return ERROR_WEAVER_CREATION_FAILED;
}

// Step 6: Configure weaver
weaver->setInputViewTexture(
    viewTextureSRV,        // Shader resource view of stereo texture
    viewWidth,             // Width of single eye view
    viewHeight,            // Height
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
);

// Optional: Enable sRGB conversion if input is sRGB but format is linear
weaver->setShaderSRGBConversion(true, true);

// Optional: Set latency (default is 2 frames)
weaver->setLatencyInFrames(2);

// Optional: Enable late latching for lower latency
weaver->enableLateLatching(true);

// Step 7: Setup system event listener for context lifecycle
class MySystemEventListener : public SR::SystemEventListener {
public:
    SR::InputStream<SR::SystemEventStream> stream;

    virtual void accept(const SR::SystemEvent& event) override {
        if (event.eventType == SR_eventType::ContextInvalid) {
            // Context lost - need to recreate SR objects
            g_reCreateContext = true;
        }
    }
};

MySystemEventListener* listener = new MySystemEventListener();
SR::SystemSense* systemSense = SR::SystemSense::create(*g_srContext);
listener->stream.set(systemSense->openSystemEventStream(listener));

// Step 8: Initialize context AFTER creating weaver
g_srContext->initialize();
```

---

## Rendering Loop

### Stereo 3D Rendering

```cpp
void Render()
{
    // Get predicted eye positions from weaver
    float leftEye[3], rightEye[3];
    weaver->getPredictedEyePositions(leftEye, rightEye);

    // Clear stereo view texture (2× width)
    const FLOAT clearColor[4] = {0.05f, 0.05f, 0.25f, 1.0f};
    d3d11Context->ClearRenderTargetView(viewTextureRTV, clearColor);
    d3d11Context->ClearDepthStencilView(viewDepthDSV,
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Set stereo view texture as render target
    d3d11Context->OMSetRenderTargets(1, &viewTextureRTV, viewDepthDSV);

    // Render both eyes side-by-side
    for (int i = 0; i < 2; i++)
    {
        // Set viewport to left or right half of texture
        D3D11_VIEWPORT viewport;
        viewport.TopLeftX = (FLOAT)(i * viewWidth);  // 0 for left, viewWidth for right
        viewport.TopLeftY = 0.0f;
        viewport.Width    = (FLOAT)viewWidth;
        viewport.Height   = (FLOAT)viewHeight;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        d3d11Context->RSSetViewports(1, &viewport);

        // Calculate view-projection matrix for this eye
        vec3f eyePos = (i == 0) ? leftEye : rightEye;
        mat4f viewProj = CalculateViewProjectionMatrix(eyePos);

        // Render scene with this eye's perspective
        RenderScene(viewProj);
    }

    // Set swapchain backbuffer as render target
    d3d11Context->OMSetRenderTargets(1, &backbufferRTV, nullptr);

    // Set viewport to full window
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width    = (FLOAT)windowWidth;
    viewport.Height   = (FLOAT)windowHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    d3d11Context->RSSetViewports(1, &viewport);

    // Perform weaving - outputs to current render target
    weaver->weave();

    // Present to screen
    swapChain->Present(1, 0); // 1 = vsync
}
```

### Generalized Perspective Projection

SR displays require **generalized perspective projection** (Kooima 2009) instead of standard symmetric frustum:

```cpp
mat4f CalculateViewProjectionMatrix(const vec3f& eye)
{
    const float znear = 0.1f;
    const float zfar  = 10000.0f;

    // Screen corners in world space (origin at screen center)
    vec3f pa = vec3f(-screenWidthMM / 2.0f,  screenHeightMM / 2.0f, 0.0f);  // Top-left
    vec3f pb = vec3f( screenWidthMM / 2.0f,  screenHeightMM / 2.0f, 0.0f);  // Top-right
    vec3f pc = vec3f(-screenWidthMM / 2.0f, -screenHeightMM / 2.0f, 0.0f);  // Bottom-left

    // Screen basis vectors
    vec3f vr = vec3f(1.0f, 0.0f, 0.0f);  // Right
    vec3f vu = vec3f(0.0f, 1.0f, 0.0f);  // Up
    vec3f vn = vec3f(0.0f, 0.0f, 1.0f);  // Normal (toward viewer)

    // Vectors from eye to screen corners
    vec3f va = pa - eye;
    vec3f vb = pb - eye;
    vec3f vc = pc - eye;

    // Distance from eye to screen plane
    float distance = -vec3f::dot(va, vn);

    // Frustum extents at near plane
    float l = vec3f::dot(vr, va) * znear / distance;
    float r = vec3f::dot(vr, vb) * znear / distance;
    float b = vec3f::dot(vu, vc) * znear / distance;
    float t = vec3f::dot(vu, va) * znear / distance;

    // Create asymmetric frustum
    mat4f frustum = mat4f::perspective(l, r, b, t, znear, zfar);

    // Translate to move eye to origin
    mat4f translate = mat4f::translation(-eye);

    return frustum * translate;
}
```

---

## Side-by-Side Image Loading

For displaying pre-rendered stereo images:

```cpp
// Load stereo image using stb_image
int imageWidth, imageHeight, channels;
unsigned char* imageData = stbi_load("StereoImage.jpg",
    &imageWidth, &imageHeight, &channels, 4);

if (imageData == nullptr) {
    // Failed to load image
    return ERROR_IMAGE_LOAD_FAILED;
}

// Create texture from image data
D3D11_SUBRESOURCE_DATA initData = {};
initData.pSysMem      = imageData;
initData.SysMemPitch  = imageWidth * 4;

D3D11_TEXTURE2D_DESC textureDesc = {};
textureDesc.Width            = imageWidth;
textureDesc.Height           = imageHeight;
textureDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
textureDesc.MipLevels        = 1;
textureDesc.ArraySize        = 1;
textureDesc.SampleDesc.Count = 1;
textureDesc.Usage            = D3D11_USAGE_DEFAULT;
textureDesc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

ID3D11Texture2D* texture = nullptr;
g_device->CreateTexture2D(&textureDesc, &initData, &texture);

ID3D11ShaderResourceView* srv = nullptr;
g_device->CreateShaderResourceView(texture, nullptr, &srv);

// Free image data
stbi_image_free(imageData);

// Set as weaver input (width/2 because it's side-by-side)
weaver->setInputViewTexture(srv, imageWidth / 2, imageHeight,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

// In render loop, just call weave() - no scene rendering needed
void Render()
{
    d3d11Context->ClearRenderTargetView(backbufferRTV, clearColor);
    d3d11Context->OMSetRenderTargets(1, &backbufferRTV, nullptr);

    // Weave the stereo image
    weaver->weave();

    // Present
    swapChain->Present(1, 0);
}
```

---

## Context Lifecycle Management

### Handling Context Invalidation

When the SR service crashes or restarts, you must recreate all SR objects:

```cpp
void HandleContextInvalidation()
{
    std::lock_guard<std::mutex> lock(g_mutex);

    // 1. Destroy weaver first (depends on context)
    if (weaver != nullptr) {
        weaver->destroy();
        weaver = nullptr;
    }

    // 2. Delete system event listener
    if (systemEventListener != nullptr) {
        delete systemEventListener;
        systemEventListener = nullptr;
    }

    // 3. Delete context
    if (g_srContext != nullptr) {
        SR::SRContext::deleteSRContext(g_srContext);
        g_srContext = nullptr;
    }

    // 4. Recreate everything with longer timeout (service is recovering)
    InitializeSR(30.0); // 30 second timeout
}
```

### Main Loop with Context Recovery

```cpp
bool g_reCreateContext = false;
std::mutex g_contextMutex;

int main()
{
    // Initial setup
    InitializeGraphics();
    InitializeSR(10.0);

    // Main loop
    while (!quit)
    {
        // Process Windows messages
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                quit = true;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Check if context needs recreation
        {
            std::lock_guard<std::mutex> lock(g_contextMutex);
            if (g_reCreateContext) {
                g_reCreateContext = false;
                HandleContextInvalidation();
            }
        }

        // Render
        if (!quit) {
            Render();
        }
    }

    // Cleanup
    Shutdown();
    return 0;
}
```

---

## DirectX 12 Differences

DirectX 12 uses a **command list-based API** instead of immediate context:

### DX12 Weaver Creation

```cpp
SR::IDX12Weaver1* weaver = nullptr;
WeaverErrorCode result = SR::CreateDX12Weaver(
    context,               // SR::SRContext*
    d3d12Device,          // ID3D12Device*
    hWnd,                 // HWND
    &weaver
);
```

### DX12 Weaver Configuration

```cpp
// Set input texture
weaver->setInputViewTexture(
    viewTexture,          // ID3D12Resource* (not SRV!)
    viewWidth,
    viewHeight,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
);

// Set output format
weaver->setOutputFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
```

### DX12 Rendering Loop

```cpp
void Render()
{
    // Reset command list
    commandList->Reset(commandAllocator[frameIndex], nullptr);

    // Render stereo views to view texture
    // ... (similar to DX11)

    // Set weaver state BEFORE calling weave()
    weaver->setCommandList(commandList);
    weaver->setViewport(viewport);
    weaver->setScissorRect(scissorRect);

    // Perform weaving
    weaver->weave();

    // Close and execute command list
    commandList->Close();
    ID3D12CommandList* commandLists[] = { commandList };
    commandQueue->ExecuteCommandLists(1, commandLists);

    // Present
    swapChain->Present(1, 0);

    // Wait for GPU
    WaitForGPU();
}
```

**Key Differences:**
- DX12 weaver takes `ID3D12Resource*` instead of shader resource view
- Must set command list, viewport, and scissor rect before each `weave()` call
- Command list-based instead of immediate execution

---

## Key Concepts

### Side-by-Side Layout

Input texture must be **2× width** with left and right eyes arranged horizontally:

```
┌─────────────┬─────────────┐
│             │             │
│  Left Eye   │  Right Eye  │
│  [0, w-1]   │  [w, 2w-1]  │
│             │             │
└─────────────┴─────────────┘
```

- **Total texture width**: `viewWidth * 2`
- **Total texture height**: `viewHeight`
- **Left eye pixels**: `x ∈ [0, viewWidth-1]`
- **Right eye pixels**: `x ∈ [viewWidth, 2*viewWidth-1]`

### Coordinate Systems

**SR SDK Coordinates:**
- **XRightYUpZBwd** (millimeters)
- X: Right positive
- Y: Up positive
- Z: Backward positive (away from display)

**Typical Application Coordinates:**
- May differ (e.g., XRightYDownZForward)
- Must transform when using `getDefaultViewingPosition()`

### Eye Prediction

Weavers automatically predict eye positions to compensate for rendering latency:

```cpp
// Set expected latency (in frames)
weaver->setLatencyInFrames(2);

// Or set explicit latency in microseconds
weaver->setLatency(33333); // 2 frames at 60 FPS
```

**Default latency:** 2 frames (typical for vsync + driver buffering)

### Late Latching

Reduces latency by updating eye positions for frames already submitted:

```cpp
weaver->enableLateLatching(true);
```

**Requirements:**
- Must call `weave()` once per frame
- May not work with deferred contexts (DX11)
- Not available on all graphics APIs

---

## Best Practices

### 1. Always Handle Context Invalidation

```cpp
class MySystemEventListener : public SR::SystemEventListener {
    virtual void accept(const SR::SystemEvent& event) override {
        if (event.eventType == SR_eventType::ContextInvalid) {
            // CRITICAL: Must recreate all SR objects
            g_reCreateContext = true;
        }
    }
};
```

### 2. Use Modern IDisplay Interface

```cpp
#define SRDISPLAY_LAZYBINDING
#include "sr/world/display/display.h"

// Try modern interface first
SR::IDisplay* display = SR::TryGetMainSRDisplay(context);
if (display && display->isValid()) {
    // Use modern API with getDefaultViewingPosition()
} else {
    // Fall back to legacy Display class
}
```

### 3. Initialize Context AFTER Creating Weaver

```cpp
// 1. Create context
SR::SRContext* context = SR::SRContext::create();

// 2. Create weaver
SR::CreateDX11Weaver(context, d3d11Context, hWnd, &weaver);

// 3. Initialize context LAST
context->initialize();
```

### 4. Use Recommended Texture Dimensions

```cpp
int viewWidth = display->getRecommendedViewsTextureWidth();
int viewHeight = display->getRecommendedViewsTextureHeight();

// Create texture with 2× width for stereo
CreateTexture(viewWidth * 2, viewHeight);
```

### 5. Handle sRGB Correctly

**Option A: Hardware sRGB**
```cpp
// Use sRGB texture format
DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

// No shader conversion needed
weaver->setShaderSRGBConversion(false, false);
```

**Option B: Shader sRGB**
```cpp
// Use linear texture format
DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

// Enable shader conversion
weaver->setShaderSRGBConversion(true, true);
```

### 6. Set Appropriate Latency

```cpp
// For double-buffered vsync application
weaver->setLatencyInFrames(2);

// For triple-buffered
weaver->setLatencyInFrames(3);

// For low-latency (mailbox mode)
weaver->setLatencyInFrames(1);
```

### 7. Avoid Blocking the SR Service

```cpp
// DON'T: Synchronous wait in main thread
while (!displayReady) {
    Sleep(100);
}

// DO: Asynchronous initialization or timeout
const double timeout = 10.0;
while (!displayReady && !TimedOut(timeout)) {
    ProcessMessages();
    Sleep(10);
}
```

### 8. Clean Up in Reverse Order

```cpp
// Destroy in reverse order of creation
weaver->destroy();              // 1. Weaver first (depends on context)
delete systemEventListener;     // 2. Event listener
SR::SRContext::deleteSRContext(context); // 3. Context last
```

### 9. Prevent Macro Redefinition Errors

```cpp
// If using SRDISPLAY_LAZYBINDING and NOMINMAX is defined by your build system:
#define SRDISPLAY_LAZYBINDING

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "sr/world/display/display.h"
```

### 10. Validate Display Before Use

```cpp
SR::IDisplay* display = SR::GetMainSRDisplay(context);
if (display && display->isValid()) {
    // Safe to use
    int width = display->getResolutionWidth();
} else {
    // Display not available or invalid
    HandleError();
}
```

---

## Example Code Locations

The SDK includes complete working examples:

- **DirectX 11**: `examples/directx11_weaving/`
- **DirectX 12**: `examples/directx12_weaving/`
- **DirectX 10**: `examples/directx10_weaving/`
- **DirectX 9**: `examples/directx9_weaving/`
- **OpenGL**: `examples/opengl_weaving/`

Each example demonstrates:
- SR context creation and lifecycle
- Weaver initialization
- Stereo rendering (both 3D scene and static SBS images)
- System event handling
- Context recovery

---

## Common Issues and Solutions

### Issue: Weaver creation fails

**Possible causes:**
1. SR service not running
2. No SR display connected
3. Display not ready
4. Context not created properly

**Solution:**
```cpp
// Wait for display to be ready
while (!displayReady) {
    SR::Display* display = SR::Display::create(*context);
    SR_recti location = display->getLocation();
    if (location.right - location.left > 0 &&
        location.bottom - location.top > 0) {
        displayReady = true;
    }
    Sleep(100);
}
```

### Issue: C4005/C2220 macro redefinition error

**Cause:** `NOMINMAX` or `WIN32_LEAN_AND_MEAN` defined both on command line and in SDK headers

**Solution:**
```cpp
#define SRDISPLAY_LAZYBINDING

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "sr/world/display/display.h"
```

### Issue: Black screen or no 3D effect

**Possible causes:**
1. Eyes not rendered correctly (wrong viewport)
2. Incorrect side-by-side layout
3. sRGB mismatch
4. Wrong texture dimensions

**Solution:**
```cpp
// Verify viewport for each eye
for (int i = 0; i < 2; i++) {
    viewport.TopLeftX = (FLOAT)(i * viewWidth);  // Critical!
    viewport.Width = (FLOAT)viewWidth;
    // Render scene
}

// Verify texture dimensions
assert(textureWidth == viewWidth * 2);  // Must be 2× width!
```

### Issue: Weaver returns nullptr from TryGetMainSRDisplay

**Cause:** Runtime version < 1.34.8 doesn't support IDisplay interface

**Solution:**
```cpp
SR::IDisplay* display = SR::TryGetMainSRDisplay(context);
if (display == nullptr) {
    // Fall back to legacy API
    SR::Display* legacyDisplay = SR::Display::create(context);
    // Use legacy API (no getDefaultViewingPosition available)
}
```

---

## Additional Resources

- **SDK Documentation**: `docs/` directory
- **Header Files**: `include/sr/` directory
- **Example Projects**: `examples/` directory
- **Libraries**: `lib/x64/` directory

---

**Document Version:** 1.0
**Last Updated:** 2025-01-15
