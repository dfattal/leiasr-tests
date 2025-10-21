# Gaussian Splat Viewer for Leia SR - Product Requirements Document

**Version:** 1.0
**Date:** 2025-01-21
**Target Platform:** Windows 10/11 x64
**SDK:** Leia SR SDK 1.34.8.31132
**Graphics API:** DirectX 11

---

## 1. Overview

A native Windows application for viewing 3D Gaussian Splat scenes (`.ply` format) on Leia SR autostereoscopic displays. The viewer provides interactive orbit camera controls and can toggle between point cloud and full splat rendering modes.

### Key Features
- ✅ Load Gaussian Splat `.ply` files
- ✅ Render splats using DirectX 11 compute shaders
- ✅ Orbit camera controls (right mouse drag)
- ✅ Toggle between point cloud and splat rendering (keyboard shortcut)
- ✅ Splat origin at world (0, 0, 0)
- ✅ Proper stereo rendering for Leia SR display
- ✅ Per-eye view-dependent splat sorting

---

## 2. Goals

### Primary Goals
1. **PLY Loading**: Parse standard Gaussian Splat `.ply` files with positions, normals, scale, rotation (quaternion), opacity, and spherical harmonics coefficients
2. **Gaussian Rendering**: Implement GPU-accelerated Gaussian splat rendering with proper alpha blending
3. **Orbit Controls**: Smooth camera orbiting around splat origin using right mouse drag
4. **Mode Switching**: Instant toggle between point cloud (simple dots) and full splat rendering
5. **SR Integration**: Proper stereo rendering with eye tracking for Leia SR display

### Secondary Goals
- Performance: Maintain 60 FPS for scenes with up to 1M splats
- Quality: Accurate Gaussian evaluation with spherical harmonics support
- Usability: Intuitive camera controls with scroll wheel zoom

---

## 3. Technical Architecture

### 3.1 Core Components

#### **SplatLoader**
- Parse `.ply` file header and binary data
- Extract per-splat attributes:
  - Position (x, y, z)
  - Normal (nx, ny, nz) - optional
  - Scale (scale_0, scale_1, scale_2)
  - Rotation quaternion (rot_0, rot_1, rot_2, rot_3)
  - Opacity (opacity)
  - Spherical harmonics coefficients (f_dc_0..2, f_rest_0..44) - optional
  - Color (red, green, blue) - fallback if no SH
- Store in GPU-friendly format (structured buffer)

#### **SplatRenderer**
- **Point Cloud Mode**: Render splats as simple colored points
  - Vertex shader: Project splat center to screen
  - Pixel shader: Output splat color

- **Splat Mode**: Render as 2D Gaussians with proper blending
  - Compute shader: Sort splats by depth (per eye)
  - Vertex shader: Generate quad billboards for each splat
  - Geometry shader: Expand point to quad with proper UV coordinates
  - Pixel shader: Evaluate 2D Gaussian, apply spherical harmonics, output RGBA

#### **OrbitCamera**
- Camera positioned in spherical coordinates (azimuth, elevation, distance)
- Origin point: (0, 0, 0)
- Controls:
  - Right mouse drag: Rotate azimuth/elevation
  - Mouse wheel: Zoom in/out (adjust distance)
  - Optional: WASD for panning origin point
- Generate view matrix from spherical coordinates
- Separate view matrices for left/right eyes using SR eye positions

#### **SplatScene**
- Manages loaded splat data
- Maintains GPU buffers (vertex buffer, index buffer, structured buffers)
- Handles rendering mode toggle
- Performs per-frame depth sorting

---

## 4. PLY File Format

### Standard Gaussian Splat PLY Structure

```
ply
format binary_little_endian 1.0
element vertex <N>
property float x
property float y
property float z
property float nx
property float ny
property float nz
property float f_dc_0
property float f_dc_1
property float f_dc_2
property float f_rest_0
...
property float f_rest_44
property float opacity
property float scale_0
property float scale_1
property float scale_2
property float rot_0
property float rot_1
property float rot_2
property float rot_3
end_header
<binary data>
```

### Required Attributes
- **Position**: `x, y, z` (float, mm)
- **Scale**: `scale_0, scale_1, scale_2` (float, log scale)
- **Rotation**: `rot_0, rot_1, rot_2, rot_3` (float, normalized quaternion)
- **Opacity**: `opacity` (float, logit space)

### Optional Attributes
- **Color**: `red, green, blue` (uint8 or float)
- **Spherical Harmonics**: `f_dc_0..2` (DC component), `f_rest_0..44` (3 bands)
- **Normal**: `nx, ny, nz` (float, for SfM point cloud)

---

## 5. Rendering Pipeline

### 5.1 Point Cloud Mode

**Vertex Shader**:
```hlsl
cbuffer PerFrame : register(b0) {
    matrix viewProj;
};

struct VS_INPUT {
    float3 position : POSITION;
    float3 color : COLOR;
};

struct PS_INPUT {
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;
    output.position = mul(viewProj, float4(input.position, 1.0));
    output.color = input.color;
    return output;
}
```

**Pixel Shader**:
```hlsl
float4 main(PS_INPUT input) : SV_TARGET {
    return float4(input.color, 1.0);
}
```

### 5.2 Splat Mode

**Compute Shader** (Depth Sort):
```hlsl
// Per-eye sorting
struct SplatDepth {
    uint index;
    float depth;
};

RWStructuredBuffer<SplatDepth> splatDepths : register(u0);

[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint idx = dispatchThreadID.x;
    // Calculate depth from eye position
    // Store in splatDepths buffer
    // Later: BitonicSort or RadixSort
}
```

**Vertex Shader** (Billboard Generation):
```hlsl
struct Splat {
    float3 position;
    float3 scale;
    float4 rotation; // quaternion
    float opacity;
    float3 sh_dc;
    // ... SH coefficients
};

StructuredBuffer<Splat> splats : register(t0);
StructuredBuffer<uint> sortedIndices : register(t1);

struct VS_OUTPUT {
    float3 centerWorld : POSITION;
    float3 scale : SCALE;
    float4 rotation : ROTATION;
    float opacity : OPACITY;
    float3 color : COLOR;
};

VS_OUTPUT main(uint vertexID : SV_VertexID) {
    uint splatIdx = sortedIndices[vertexID];
    Splat splat = splats[splatIdx];

    VS_OUTPUT output;
    output.centerWorld = splat.position;
    output.scale = exp(splat.scale); // Convert from log space
    output.rotation = splat.rotation;
    output.opacity = 1.0 / (1.0 + exp(-splat.opacity)); // Sigmoid
    output.color = evaluateSphericalHarmonics(splat, viewDir);
    return output;
}
```

**Geometry Shader** (Quad Expansion):
```hlsl
[maxvertexcount(4)]
void main(point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> triStream) {
    // Project 3D Gaussian covariance to 2D screen space
    float3x3 covariance3D = computeCovariance3D(input[0].scale, input[0].rotation);
    float3x3 viewMat = ...; // From constant buffer
    float3x3 covariance2D = projectCovariance(covariance3D, viewMat, ...);

    // Compute 2D bounding quad
    float2 v1 = ..., v2 = ...; // Eigenvectors of covariance2D

    // Emit 4 vertices forming a quad
    // Pass covariance2D inverse for pixel shader Gaussian evaluation
}
```

**Pixel Shader** (Gaussian Evaluation):
```hlsl
struct PS_INPUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0; // [-1,1] quad space
    float3 color : COLOR;
    float opacity : OPACITY;
    float2x2 covInv : COVINV; // Inverse of 2D covariance
};

float4 main(PS_INPUT input) : SV_TARGET {
    // Evaluate 2D Gaussian
    float2 d = input.uv;
    float power = -0.5 * dot(d, mul(input.covInv, d));
    float alpha = input.opacity * exp(power);

    // Pre-multiplied alpha for correct blending
    return float4(input.color * alpha, alpha);
}
```

**Blend State**:
- Blend enabled
- SrcBlend = One
- DestBlend = InvSrcAlpha
- BlendOp = Add
- Render back-to-front (requires sorting)

---

## 6. Camera System

### 6.1 Orbit Camera Math

**Spherical Coordinates**:
- Azimuth (θ): Rotation around Y-axis (horizontal)
- Elevation (φ): Angle from XZ-plane (vertical)
- Distance (r): Radius from origin

**Cartesian Conversion**:
```cpp
vec3f cameraPosition;
cameraPosition.x = distance * cos(elevation) * sin(azimuth);
cameraPosition.y = distance * sin(elevation);
cameraPosition.z = distance * cos(elevation) * cos(azimuth);
```

**View Matrix**:
```cpp
vec3f origin(0, 0, 0);
vec3f up(0, 1, 0);
mat4f viewMatrix = LookAt(cameraPosition, origin, up);
```

### 6.2 Controls

| Input | Action | Sensitivity |
|-------|--------|-------------|
| Right Mouse Drag X | Adjust azimuth | 0.005 rad/pixel |
| Right Mouse Drag Y | Adjust elevation | 0.005 rad/pixel |
| Mouse Wheel Up | Decrease distance (zoom in) | -10% per tick |
| Mouse Wheel Down | Increase distance (zoom out) | +10% per tick |
| 'C' Key | Toggle point cloud / splat mode | Instant |
| 'R' Key | Reset camera to default position | Instant |
| Esc | Exit application | Immediate |

**Constraints**:
- Elevation: Clamp to [-89°, +89°] (avoid gimbal lock)
- Distance: Clamp to [100mm, 10000mm]
- Azimuth: Wrap around [0, 2π]

---

## 7. Stereo Rendering for Leia SR

### 7.1 Eye-Dependent Rendering

```cpp
// Get eye positions from SR SDK
vec3f leftEyePos, rightEyePos;
eyeTracker->getEyePositions(leftEyePos, rightEyePos);

// Compute per-eye camera positions
vec3f orbitCenter(0, 0, 0);
vec3f orbitOffset = cameraPosition - orbitCenter;

vec3f leftCameraPos = orbitCenter + orbitOffset + (leftEyePos - defaultViewPos);
vec3f rightCameraPos = orbitCenter + orbitOffset + (rightEyePos - defaultViewPos);

// Render each eye separately
for (int eye = 0; eye < 2; eye++) {
    vec3f eyeCameraPos = (eye == 0) ? leftCameraPos : rightCameraPos;

    // Sort splats by depth from this eye position
    sortSplatsByDepth(splats, eyeCameraPos);

    // Render splats back-to-front
    renderSplats(eyeCameraPos);
}
```

### 7.2 View-Dependent Spherical Harmonics

Spherical harmonics coefficients must be evaluated per-eye based on view direction:

```hlsl
float3 evaluateSH(Splat splat, float3 viewDir) {
    float3 color = splat.sh_dc; // DC component (constant)

    // Band 1 (linear)
    color += splat.sh_1_0 * viewDir.y;
    color += splat.sh_1_1 * viewDir.z;
    color += splat.sh_1_2 * viewDir.x;

    // Band 2 (quadratic) - if available
    // ... (see reference implementation)

    return max(color, 0.0); // Clamp to positive
}
```

---

## 8. Performance Targets

### 8.1 Metrics

| Metric | Target | Acceptable |
|--------|--------|------------|
| FPS (100K splats) | 60 | 45 |
| FPS (500K splats) | 60 | 30 |
| FPS (1M splats) | 45 | 24 |
| PLY Load Time (1M splats) | < 2s | < 5s |
| Sort Time per Eye | < 5ms | < 10ms |
| Memory Usage (1M splats) | < 500MB | < 1GB |

### 8.2 Optimizations

1. **GPU Sorting**: Use BitonicSort or RadixSort compute shader
2. **Frustum Culling**: Cull splats outside view frustum before sorting
3. **LOD**: Skip sorting/rendering for splats with projected size < 1 pixel
4. **Instancing**: Use DrawInstanced for splat quads
5. **Persistent Buffers**: Reuse GPU buffers across frames

---

## 9. User Interface

### 9.1 HUD Overlay

```
┌─────────────────────────────────┐
│ Gaussian Splat Viewer           │
│ File: scene.ply                 │
│ Splats: 1,234,567               │
│ Mode: [SPLAT RENDERING]         │
│ FPS: 60.0                       │
│ Camera: Az=45° El=30° Dist=500mm│
└─────────────────────────────────┘
```

### 9.2 Controls Help (toggled with 'H')

```
Controls:
  Right Mouse Drag - Orbit camera
  Mouse Wheel      - Zoom in/out
  C                - Toggle point cloud / splat
  R                - Reset camera
  H                - Toggle this help
  Esc              - Exit
```

---

## 10. File Structure

```
LeiaSplatViewer/
├── src/
│   ├── main.cpp                 // Application entry, window, main loop
│   ├── Math.h                   // Vector/matrix utilities (reuse from LookAroundEyes)
│   ├── SplatLoader.h/cpp        // PLY file parsing
│   ├── SplatRenderer.h/cpp      // Rendering pipeline
│   ├── OrbitCamera.h/cpp        // Camera controls
│   ├── SplatScene.h/cpp         // Scene management
│   └── HUD.h/cpp                // Direct2D overlay
├── shaders/
│   ├── PointCloud_VS.hlsl       // Point cloud vertex shader
│   ├── PointCloud_PS.hlsl       // Point cloud pixel shader
│   ├── Splat_VS.hlsl            // Splat vertex shader
│   ├── Splat_GS.hlsl            // Splat geometry shader (quad expansion)
│   ├── Splat_PS.hlsl            // Splat pixel shader (Gaussian eval)
│   └── SplatSort_CS.hlsl        // Depth sorting compute shader
├── LeiaSplatViewer.vcxproj
├── README.md
├── BUILD_INSTRUCTIONS.md
└── test_data/
    └── sample.ply               // Test splat scene
```

---

## 11. Implementation Phases

### Phase 1: Foundation (Core Infrastructure)
- [x] Project setup (Visual Studio, DirectX 11)
- [ ] PLY file parser (positions, colors only)
- [ ] Point cloud rendering (simple dots)
- [ ] Orbit camera controls
- [ ] SR context initialization

### Phase 2: Basic Splat Rendering
- [ ] Parse full splat attributes (scale, rotation, opacity)
- [ ] Implement splat vertex/geometry/pixel shaders
- [ ] 2D Gaussian evaluation
- [ ] Alpha blending setup
- [ ] Per-eye depth sorting (CPU)

### Phase 3: Quality & Performance
- [ ] Spherical harmonics support
- [ ] GPU-based sorting (compute shader)
- [ ] Frustum culling
- [ ] Mode toggle (point cloud ↔ splat)
- [ ] HUD overlay

### Phase 4: Polish
- [ ] Camera reset, constraints
- [ ] Help overlay
- [ ] Error handling (invalid PLY)
- [ ] Performance profiling
- [ ] Documentation

---

## 12. Test Cases

### 12.1 Functional Tests

1. **PLY Loading**:
   - Load standard Gaussian Splat PLY (positions, SH, scale, rotation, opacity)
   - Load minimal PLY (positions, colors only)
   - Handle invalid PLY gracefully (show error message)

2. **Rendering Modes**:
   - Point cloud mode shows colored dots at splat positions
   - Splat mode shows smooth Gaussian blobs
   - Toggle between modes is instant (no lag)

3. **Camera Controls**:
   - Right drag rotates camera around origin
   - Scroll wheel zooms in/out
   - Camera constraints work (elevation clamp, distance clamp)
   - Reset returns to default position

4. **Stereo**:
   - Left/right eyes render from slightly different positions
   - 3D depth perception is correct on SR display
   - No visual artifacts during camera movement

### 12.2 Performance Tests

- 100K splats at 60 FPS
- 500K splats at 30+ FPS
- Memory usage within budget
- Smooth camera rotation (no stutter)

---

## 13. Known Limitations

1. **File Format**: Only `.ply` binary little-endian format supported
2. **Sorting**: Per-frame sorting limits scalability (>1M splats may drop FPS)
3. **SH Bands**: Maximum 3 bands (DC + 3 orders) supported
4. **Single Scene**: No multi-scene loading or switching
5. **No Editing**: View-only, no splat manipulation

---

## 14. Future Enhancements

- **Compression**: Support compressed `.splat` format for faster loading
- **Streaming**: Level-of-detail streaming for massive scenes (>10M splats)
- **Multi-Scene**: Load multiple PLY files, switch between them
- **Export**: Save camera paths, screenshots
- **VR Input**: Support VR controllers for interaction (if Leia SR supports)

---

## 15. References

- **Splatapult**: https://github.com/hyperlogic/splatapult - C++/OpenGL reference implementation
- **3D Gaussian Splatting Paper**: "3D Gaussian Splatting for Real-Time Radiance Field Rendering" (Kerbl et al., 2023)
- **PLY Format**: http://paulbourke.net/dataformats/ply/
- **Leia SR SDK**: 1.34.8.31132 documentation
- **Kooima Projection**: "Generalized Perspective Projection" (2009) - for off-axis stereo

---

**Version History**:
- v1.0 (2025-01-21): Initial PRD based on Splatapult reference and Leia SR SDK requirements
