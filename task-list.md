# LookAround Eyes Comparison App - Task List

**Project:** Eye Tracking Method Comparison Tool
**Started:** 2025-01-15
**Status:** In Progress

---

## Phase 1: Project Setup ✓

- [x] Create PRD document
- [x] Create task list
- [ ] Create Visual Studio project structure
- [ ] Configure project properties and dependencies
- [ ] Copy required SDK headers and libraries

---

## Phase 2: Core Infrastructure

### 2.1 Math Utilities
- [ ] Create `Math.h` with vec3f, vec4f, mat4f structures
- [ ] Implement matrix operations (translation, perspective, multiply)
- [ ] Implement vector operations (dot, cross, normalize)

### 2.2 Window & DirectX Setup
- [ ] Implement window creation (WndProc, message loop)
- [ ] Initialize DirectX 11 device and context
- [ ] Create swap chain
- [ ] Implement fullscreen toggle (F11)
- [ ] DPI awareness setup

### 2.3 SR Context Initialization
- [ ] Create SR context with retry logic
- [ ] Query IDisplay interface with lazy binding
- [ ] Get recommended texture dimensions
- [ ] Get physical screen size and default viewing position
- [ ] Calculate FOV from viewing pyramid
- [ ] Create stereo view texture (2× width)
- [ ] Initialize weaver

---

## Phase 3: Dual Eye Tracking System

### 3.1 Eye Tracker Classes
- [ ] Implement `SREyePairListener` class
  - [ ] Accept method for SR_eyePair callbacks
  - [ ] Store left/right eye positions
  - [ ] Create eye pair stream
- [ ] Implement `DualEyeTracker` class
  - [ ] Constructor: initialize both tracking methods
  - [ ] `setMode()`: switch between Weaver/Listener
  - [ ] `getMode()`: return current mode
  - [ ] `getEyePositions()`: return eyes based on active mode
  - [ ] Destructor: cleanup resources

### 3.2 System Event Handling
- [ ] Implement `SRSystemEventListener` class
- [ ] Handle `ContextInvalid` event
- [ ] Setup context recovery mechanism

---

## Phase 4: 3D Scene Rendering

### 4.1 Shader Creation
- [ ] Write vertex shader (VertexShader.hlsl)
- [ ] Write pixel shader (PixelShader.hlsl)
- [ ] Compile shaders at runtime
- [ ] Create shader resource views

### 4.2 Geometry Generation
- [ ] Create cube vertex/index buffers
- [ ] Create sphere vertex/index buffers (icosphere or UV sphere)
- [ ] Implement `CreateCubeGeometry()` helper
- [ ] Implement `CreateSphereGeometry()` helper

### 4.3 Scene Class
- [ ] Implement `Scene::initialize()`
  - [ ] Store screen dimensions and virtual screen depth
  - [ ] Create constant buffer
  - [ ] Create near plane objects (red cube at 200mm)
  - [ ] Create mid plane objects (green/blue spheres at 400mm)
  - [ ] Create far plane objects (yellow cube at 700mm)
  - [ ] Create background grid (optional, at 1000mm)
- [ ] Implement `Scene::render()`
  - [ ] Calculate view-projection matrix with virtual screen at 400mm
  - [ ] Update constant buffer
  - [ ] Render each object with rotation

### 4.4 View-Projection Matrix
- [ ] Implement `CalculateViewProjectionMatrix()`
  - [ ] Use generalized perspective projection (Kooima 2009)
  - [ ] Position screen corners at virtualScreenDepthMM (400mm)
  - [ ] Calculate asymmetric frustum
  - [ ] Apply eye translation

---

## Phase 5: HUD Overlay

### 5.1 Direct2D/DirectWrite Setup
- [ ] Initialize Direct2D factory
- [ ] Initialize DirectWrite factory
- [ ] Create render target for backbuffer
- [ ] Create text format (font, size)
- [ ] Create brushes (white, green)

### 5.2 HUD Class
- [ ] Implement `HUD::initialize()`
- [ ] Implement `HUD::render()`
  - [ ] Display current eye tracking mode
  - [ ] Display FPS counter
  - [ ] Semi-transparent background
- [ ] Implement FPS calculation
  - [ ] Update every 250ms
  - [ ] Running average

---

## Phase 6: Input Handling

- [ ] Implement keyboard input in WndProc
  - [ ] Tab: Toggle eye tracking mode
  - [ ] F11: Toggle fullscreen
  - [ ] Esc: Exit application
  - [ ] C: Simulate context invalidation
- [ ] Add visual/audio feedback for mode switch (optional)

---

## Phase 7: Main Render Loop

- [ ] Implement main render function
  - [ ] Get eye positions from DualEyeTracker
  - [ ] Clear stereo view texture
  - [ ] Render left eye (viewport 0 to viewWidth)
  - [ ] Render right eye (viewport viewWidth to 2×viewWidth)
  - [ ] Set backbuffer as render target
  - [ ] Perform weaving
  - [ ] Render HUD overlay
  - [ ] Present
- [ ] Implement timing/FPS tracking
- [ ] Handle window resize

---

## Phase 8: Resource Management

### 8.1 Initialization Order
- [ ] Ensure correct initialization sequence per PRD
  - [ ] Create context
  - [ ] Wait for display ready
  - [ ] Create DirectX resources
  - [ ] Create weaver
  - [ ] Initialize dual eye tracker
  - [ ] Initialize scene
  - [ ] Initialize HUD
  - [ ] **Call context->initialize() LAST**

### 8.2 Cleanup
- [ ] Implement proper shutdown sequence
  - [ ] Destroy weaver first
  - [ ] Delete listeners
  - [ ] Delete SR context
  - [ ] Release DirectX resources
- [ ] Check for memory leaks

---

## Phase 9: Testing & Validation

### 9.1 Functional Testing
- [ ] Test application launch on Leia display
- [ ] Verify 3D scene renders correctly
- [ ] Test Tab key mode switching
- [ ] Verify HUD displays correctly
- [ ] Test FPS counter accuracy
- [ ] Verify all 5 objects at correct depths
- [ ] Test F11 fullscreen toggle
- [ ] Test Esc exit

### 9.2 Comparison Testing
- [ ] Compare visual quality between modes
- [ ] Test rapid mode switching (Tab spam)
- [ ] Move head while in each mode
- [ ] Observe parallax differences
- [ ] Check depth perception at each plane
- [ ] Verify 60 FPS in both modes

### 9.3 Edge Cases
- [ ] Test context invalidation recovery (C key)
- [ ] Test extended runtime (5+ minutes each mode)
- [ ] Test with window minimized
- [ ] Test window resizing

### 9.4 Performance
- [ ] Profile with Visual Studio
- [ ] Check for memory leaks
- [ ] Verify GPU usage
- [ ] Check CPU usage

---

## Phase 10: Documentation & Cleanup

- [ ] Add code comments
- [ ] Document any deviations from PRD
- [ ] Create build instructions (README)
- [ ] Document findings from mode comparison
- [ ] Clean up debug code
- [ ] Final code review

---

## Current Status

**Active Phase:** Phase 2 - Core Infrastructure
**Next Task:** Create Visual Studio project structure

---

## Notes

- Virtual screen depth set at 400mm to focus mid-plane objects
- FOV derived from IDisplay physical size / default viewing position
- Texture dimensions from IDisplay getRecommendedViewsTexture*()
- Both eye tracking methods active simultaneously for instant switching
