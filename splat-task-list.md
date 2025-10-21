# Gaussian Splat Viewer - Implementation Task List

**Project**: LeiaSplatViewer
**Start Date**: 2025-01-21
**Target**: Complete functional Gaussian Splat viewer for Leia SR

---

## Phase 1: Foundation & Project Setup

### 1.1 Project Structure
- [ ] Create `LeiaSplatViewer/` directory
- [ ] Create Visual Studio project file (`LeiaSplatViewer.vcxproj`)
- [ ] Set up directory structure (src/, shaders/, test_data/)
- [ ] Configure include paths for Leia SR SDK
- [ ] Configure library paths and link dependencies
- [ ] Set up post-build events (copy DLLs)

### 1.2 Core Math Utilities
- [ ] Copy/adapt `Math.h` from LookAroundEyesApp
- [ ] Add quaternion support (for splat rotation)
- [ ] Add covariance matrix utilities
- [ ] Add spherical coordinate conversion

### 1.3 Window & DirectX Initialization
- [ ] Create main.cpp with WinMain entry point
- [ ] Window creation and message loop
- [ ] DirectX 11 device and swap chain creation
- [ ] Render target view setup
- [ ] Depth stencil view setup
- [ ] Viewport configuration

### 1.4 Leia SR Integration
- [ ] SR context initialization
- [ ] SR display detection
- [ ] SR weaver creation
- [ ] Eye tracker setup (reuse from LookAroundEyesApp)
- [ ] Side-by-side stereo texture setup
- [ ] View texture dimensions from IDisplay

---

## Phase 2: PLY File Parsing

### 2.1 SplatLoader Class
- [ ] Create `SplatLoader.h` interface
- [ ] Create `SplatLoader.cpp` implementation
- [ ] PLY header parser
  - [ ] Parse `format` line (expect binary_little_endian)
  - [ ] Parse `element vertex N` line (get splat count)
  - [ ] Parse `property` lines (build attribute map)
  - [ ] Detect `end_header` marker
- [ ] Binary data reader
  - [ ] Read vertex data based on attribute map
  - [ ] Handle different property types (float, uchar, int)
  - [ ] Store in intermediate data structure

### 2.2 Splat Data Structure
- [ ] Define `struct SplatVertex` with all attributes:
  - [ ] Position (x, y, z) - float3
  - [ ] Scale (scale_0, scale_1, scale_2) - float3, log space
  - [ ] Rotation (rot_0, rot_1, rot_2, rot_3) - float4, quaternion
  - [ ] Opacity - float, logit space
  - [ ] SH DC (f_dc_0, f_dc_1, f_dc_2) - float3
  - [ ] SH Rest (f_rest_0..44) - float[45], optional
  - [ ] Color (r, g, b) - float3, fallback if no SH
- [ ] Create `std::vector<SplatVertex>` storage
- [ ] GPU buffer creation from loaded data

### 2.3 File I/O
- [ ] Windows file dialog integration (GetOpenFileName)
- [ ] File path handling
- [ ] Error handling (file not found, invalid format)
- [ ] Progress reporting for large files

---

## Phase 3: Point Cloud Rendering (Simple Mode)

### 3.1 Point Cloud Shaders
- [ ] Create `PointCloud_VS.hlsl`:
  - [ ] Input: position, color
  - [ ] Output: transformed position, color
  - [ ] Apply view-projection matrix
- [ ] Create `PointCloud_PS.hlsl`:
  - [ ] Input: color
  - [ ] Output: solid color (no lighting)

### 3.2 Point Cloud Renderer
- [ ] Compile shaders at runtime (D3DCompileFromFile)
- [ ] Create vertex shader
- [ ] Create pixel shader
- [ ] Create input layout (position + color)
- [ ] Create constant buffer (view-projection matrix)
- [ ] Set rasterizer state (point list topology)
- [ ] Draw call (DrawInstanced or Draw)

### 3.3 Initial Rendering
- [ ] Clear render target
- [ ] Set shaders and resources
- [ ] Draw point cloud
- [ ] Present to screen
- [ ] Verify points are visible

---

## Phase 4: Orbit Camera System

### 4.1 OrbitCamera Class
- [ ] Create `OrbitCamera.h` interface
- [ ] Create `OrbitCamera.cpp` implementation
- [ ] Member variables:
  - [ ] Azimuth (float, radians)
  - [ ] Elevation (float, radians)
  - [ ] Distance (float, mm)
  - [ ] Origin point (vec3f, default 0,0,0)
- [ ] `update()` method - compute camera position from spherical coords
- [ ] `getViewMatrix()` - generate view matrix for rendering
- [ ] `getViewProjectionMatrix()` - combined VP matrix
- [ ] `reset()` - return to default position

### 4.2 Mouse Input Handling
- [ ] WM_RBUTTONDOWN - track mouse capture state
- [ ] WM_RBUTTONUP - release mouse capture
- [ ] WM_MOUSEMOVE - update azimuth/elevation when right button down
  - [ ] Delta X → azimuth change
  - [ ] Delta Y → elevation change
  - [ ] Apply sensitivity multiplier
- [ ] WM_MOUSEWHEEL - adjust distance
  - [ ] Scroll up → zoom in (decrease distance)
  - [ ] Scroll down → zoom out (increase distance)

### 4.3 Camera Constraints
- [ ] Elevation clamping: [-89°, +89°]
- [ ] Distance clamping: [100mm, 10000mm]
- [ ] Azimuth wrapping: [0, 360°]
- [ ] Smooth interpolation (optional)

### 4.4 Integration with Rendering
- [ ] Update camera each frame
- [ ] Pass view matrix to shaders
- [ ] Verify camera orbits around (0,0,0)
- [ ] Test zoom in/out

---

## Phase 5: Gaussian Splat Rendering

### 5.1 Splat Vertex Shader
- [ ] Create `Splat_VS.hlsl`:
  - [ ] Input: splat attributes (position, scale, rotation, opacity, SH)
  - [ ] Output: world position, transformed attributes
  - [ ] Convert scale from log space: `exp(scale)`
  - [ ] Convert opacity from logit: `1/(1+exp(-opacity))`
  - [ ] Pass through to geometry shader

### 5.2 Splat Geometry Shader
- [ ] Create `Splat_GS.hlsl`:
  - [ ] Input: point (splat center)
  - [ ] Output: quad (4 vertices forming billboard)
  - [ ] Compute 3D covariance matrix from scale and rotation
  - [ ] Project 3D covariance to 2D screen space
  - [ ] Compute 2D bounding quad from covariance eigenvectors
  - [ ] Emit 4 vertices with UVs in [-1, 1] range
  - [ ] Pass inverse covariance for pixel shader

### 5.3 Splat Pixel Shader
- [ ] Create `Splat_PS.hlsl`:
  - [ ] Input: UV coordinates, color, opacity, inverse covariance
  - [ ] Evaluate 2D Gaussian: `exp(-0.5 * uv^T * Σ^-1 * uv)`
  - [ ] Multiply by opacity
  - [ ] Apply spherical harmonics color based on view direction
  - [ ] Output: pre-multiplied alpha `(color * alpha, alpha)`

### 5.4 Spherical Harmonics Evaluation
- [ ] Implement SH evaluation function in pixel shader
- [ ] DC component (constant color)
- [ ] Band 1 (3 coefficients, linear in view direction)
- [ ] Band 2 (5 coefficients, quadratic) - optional
- [ ] Band 3 (7 coefficients, cubic) - optional
- [ ] Clamp result to [0, inf)

### 5.5 Render State Configuration
- [ ] Create blend state:
  - [ ] Enable alpha blending
  - [ ] SrcBlend = One (pre-multiplied alpha)
  - [ ] DestBlend = InvSrcAlpha
  - [ ] BlendOp = Add
- [ ] Create depth stencil state:
  - [ ] Depth test enabled
  - [ ] Depth write disabled (transparent rendering)
- [ ] Create rasterizer state:
  - [ ] Cull mode = None (render both sides)

### 5.6 Splat Renderer Integration
- [ ] Compile splat shaders
- [ ] Create shader objects
- [ ] Create input layout for splat vertices
- [ ] Set up structured buffer for splat data
- [ ] Bind shaders and resources
- [ ] Draw splats
- [ ] Verify basic rendering works

---

## Phase 6: Depth Sorting

### 6.1 CPU Sorting (Initial Implementation)
- [ ] For each frame, for each eye:
  - [ ] Compute eye position from camera + eye offset
  - [ ] Calculate depth of each splat from eye position
  - [ ] Create array of (index, depth) pairs
  - [ ] Sort by depth (back-to-front for alpha blending)
  - [ ] Create index buffer with sorted indices
- [ ] Bind sorted index buffer
- [ ] Use DrawIndexed instead of Draw

### 6.2 GPU Sorting (Optimization)
- [ ] Create `SplatSort_CS.hlsl` compute shader
- [ ] Implement BitonicSort algorithm:
  - [ ] Multiple passes with increasing stride
  - [ ] Compare-and-swap operations
  - [ ] Work on structured buffer of (index, depth)
- [ ] Alternative: RadixSort for better performance
- [ ] Dispatch compute shader before rendering
- [ ] Use sorted buffer for draw call

### 6.3 Per-Eye Sorting
- [ ] Sort splats separately for left eye
- [ ] Render left eye view
- [ ] Sort splats separately for right eye
- [ ] Render right eye view
- [ ] Profile sorting overhead (target < 10ms)

---

## Phase 7: Mode Toggle & UI

### 7.1 Rendering Mode Management
- [ ] Create `enum class RenderMode { PointCloud, Splat }`
- [ ] Track current mode in global state
- [ ] Switch shaders based on mode
- [ ] 'C' key handler to toggle mode
- [ ] Update HUD to show current mode

### 7.2 HUD Integration
- [ ] Reuse HUD class from LookAroundEyesApp
- [ ] Display overlay:
  - [ ] File name
  - [ ] Splat count
  - [ ] Current render mode
  - [ ] FPS
  - [ ] Camera parameters (azimuth, elevation, distance)
- [ ] Update HUD each frame

### 7.3 Help Overlay
- [ ] Create help text with controls
- [ ] 'H' key to toggle help visibility
- [ ] Render help over HUD when visible
- [ ] Format controls nicely

### 7.4 Additional Keyboard Shortcuts
- [ ] 'R' key: Reset camera to default
- [ ] 'C' key: Toggle point cloud / splat
- [ ] 'H' key: Toggle help
- [ ] 'Esc' key: Exit application
- [ ] 'F11' key: Toggle fullscreen (optional)

---

## Phase 8: Stereo Rendering for Leia SR

### 8.1 Eye Position Integration
- [ ] Get eye positions from SR eye tracker
- [ ] Calculate per-eye camera positions:
  - [ ] Orbit camera base position
  - [ ] Add eye offset from default view position
- [ ] Generate separate view matrices for left/right eyes

### 8.2 Side-by-Side Rendering
- [ ] Set viewport for left eye (left half of texture)
- [ ] Render splats from left eye position
- [ ] Set viewport for right eye (right half of texture)
- [ ] Render splats from right eye position
- [ ] Weave to backbuffer

### 8.3 View-Dependent SH
- [ ] Calculate view direction per-eye
- [ ] Evaluate SH with correct view direction
- [ ] Verify color changes are per-eye (view-dependent effects)

---

## Phase 9: Optimization & Polish

### 9.1 Performance Optimization
- [ ] Profile rendering pipeline (GPU profiler)
- [ ] Frustum culling:
  - [ ] Test splat bounding sphere against view frustum
  - [ ] Skip rendering/sorting for culled splats
- [ ] LOD system:
  - [ ] Calculate projected splat size
  - [ ] Skip splats with projected size < 1 pixel
- [ ] Instanced rendering for splat quads
- [ ] Persistent GPU buffers (avoid re-upload)

### 9.2 Memory Optimization
- [ ] Profile memory usage
- [ ] Use 16-bit floats where possible (scale, rotation)
- [ ] Compress SH coefficients if needed
- [ ] Stream large scenes in chunks (if > 1M splats)

### 9.3 Error Handling
- [ ] Validate PLY file format
- [ ] Handle missing attributes gracefully
- [ ] Show error dialog for invalid files
- [ ] Fallback to simple color if no SH coefficients
- [ ] Handle SR initialization failures

### 9.4 Camera Improvements
- [ ] Smooth camera interpolation
- [ ] Auto-frame splat bounds on load
- [ ] Save/restore camera position
- [ ] Camera animation (optional)

---

## Phase 10: Testing & Documentation

### 10.1 Test Cases
- [ ] Load small splat scene (< 10K splats)
- [ ] Load medium scene (100K splats)
- [ ] Load large scene (500K+ splats)
- [ ] Test point cloud mode
- [ ] Test splat mode
- [ ] Test mode switching
- [ ] Test camera controls (orbit, zoom, reset)
- [ ] Test on Leia SR display (stereo rendering)
- [ ] Test with different PLY formats (with/without SH)

### 10.2 Performance Testing
- [ ] Measure FPS for various splat counts
- [ ] Measure load times
- [ ] Measure sorting overhead
- [ ] Measure memory usage
- [ ] Profile GPU usage

### 10.3 Documentation
- [ ] Create README.md:
  - [ ] Overview of viewer
  - [ ] Features list
  - [ ] Controls reference
  - [ ] Build instructions
- [ ] Create BUILD_INSTRUCTIONS.md:
  - [ ] Prerequisites
  - [ ] Build steps
  - [ ] Troubleshooting
  - [ ] Running the application
- [ ] Create HOW_TO_USE.md:
  - [ ] Loading PLY files
  - [ ] Camera controls
  - [ ] Render modes
  - [ ] Performance tips
- [ ] Code comments and documentation

### 10.4 Sample Data
- [ ] Find or create sample .ply file
- [ ] Place in `test_data/sample.ply`
- [ ] Document where to find more splat scenes
- [ ] Test with LumaLabs.ai scenes

---

## Phase 11: Packaging & Deployment

### 11.1 Build Configurations
- [ ] Debug configuration
- [ ] Release configuration (optimizations enabled)
- [ ] Verify both build successfully

### 11.2 Deployment Package
- [ ] Copy executable to distribution folder
- [ ] Copy required DLLs (SR SDK, MSVC runtime)
- [ ] Copy shaders folder
- [ ] Include sample PLY file
- [ ] Create README.txt with quick start instructions

### 11.3 Final Testing
- [ ] Test on clean Windows machine
- [ ] Verify all dependencies present
- [ ] Test with various PLY files
- [ ] Performance validation

---

## Implementation Checklist Summary

### Critical Path
1. ✅ Project setup
2. ✅ PLY parser
3. ✅ Point cloud rendering
4. ✅ Orbit camera
5. ✅ Basic splat rendering (no sorting)
6. ✅ Depth sorting
7. ✅ Mode toggle
8. ✅ Stereo rendering
9. ✅ Optimization
10. ✅ Testing & documentation

### Estimated Timeline
- Phase 1-2: 2-3 hours (foundation, PLY parsing)
- Phase 3-4: 1-2 hours (point cloud, camera)
- Phase 5-6: 3-4 hours (splat rendering, sorting)
- Phase 7-8: 1-2 hours (UI, stereo)
- Phase 9-10: 2-3 hours (optimization, testing)
- Phase 11: 1 hour (packaging)

**Total: ~10-15 hours of focused development**

---

## Notes
- Reuse code from LookAroundEyesApp where possible (Math.h, HUD, SR initialization)
- Reference Splatapult for shader implementation details
- Start simple (point cloud) and iterate to full splat rendering
- Test frequently with small PLY files during development
- Profile early and often to avoid performance surprises
