# Leia Gaussian Splat Viewer

A native Windows application for viewing 3D Gaussian Splat scenes (`.ply` format) on Leia SR autostereoscopic displays with interactive orbit camera controls.

---

## Features

✅ Load Gaussian Splat `.ply` files (binary little-endian format)
✅ Orbit camera controls (right mouse drag to rotate, scroll wheel to zoom)
✅ Toggle between point cloud and Gaussian splat rendering modes
✅ Stereo rendering for Leia SR 3D displays
✅ Automatic scene framing
✅ Spherical harmonics support for view-dependent colors
✅ Real-time depth sorting for proper alpha blending

---

## Quick Start

### Prerequisites

- Windows 10/11 (x64)
- Visual Studio 2019 or later
- Leia SR SDK 1.34.8.31132
- Leia SR Platform Service installed and running
- Leia SR display connected

### Building

1. Open `LeiaSplatViewer.vcxproj` in Visual Studio
2. Select **x64** platform (top toolbar)
3. Select **Release** configuration
4. Press **F7** to build

### Running

1. Press **Ctrl+F5** to run
2. Click **Open** or press **O** to load a `.ply` file
3. Use right mouse drag to orbit camera
4. Use scroll wheel to zoom in/out
5. Press **C** to toggle between point cloud and splat rendering

---

## Controls

| Input | Action |
|-------|--------|
| **Right Mouse Drag** | Orbit camera around splat origin |
| **Mouse Wheel** | Zoom in/out |
| **C** | Toggle point cloud / Gaussian splat rendering |
| **R** | Reset camera to default position |
| **O** | Open PLY file |
| **Esc** | Exit application |

---

## File Format

### Supported PLY Format

The viewer supports standard Gaussian Splat `.ply` files with the following attributes:

**Required:**
- `x, y, z` - Position (float)
- `scale_0, scale_1, scale_2` - Scale in log space (float)
- `rot_0, rot_1, rot_2, rot_3` - Rotation quaternion (float, normalized)
- `opacity` - Opacity in logit space (float)

**Optional:**
- `f_dc_0, f_dc_1, f_dc_2` - Spherical harmonics DC component (float)
- `f_rest_0` through `f_rest_44` - SH coefficients for 3 bands (float)
- `red, green, blue` - Fallback color if no SH (uchar or float)

### Example PLY Header

```
ply
format binary_little_endian 1.0
element vertex 100000
property float x
property float y
property float z
property float scale_0
property float scale_1
property float scale_2
property float rot_0
property float rot_1
property float rot_2
property float rot_3
property float opacity
property float f_dc_0
property float f_dc_1
property float f_dc_2
...
end_header
<binary data>
```

---

## Architecture

### Core Components

**SplatLoader** (`SplatLoader.h/cpp`)
- Parses `.ply` file header and binary data
- Extracts splat attributes (position, scale, rotation, opacity, SH)
- Computes scene bounding box

**OrbitCamera** (`OrbitCamera.h/cpp`)
- Manages camera in spherical coordinates (azimuth, elevation, distance)
- Handles mouse input (drag to rotate, wheel to zoom)
- Generates view and projection matrices

**Math** (`Math.h`)
- Vector/matrix utilities (vec3f, vec4f, mat3f, mat4f)
- Quaternion operations and conversion to rotation matrix
- Covariance matrix computation
- Spherical coordinate conversion

### Rendering Pipeline

#### Point Cloud Mode
1. Load splat positions and colors
2. Render as simple points (D3D11_PRIMITIVE_TOPOLOGY_POINTLIST)
3. Vertex shader projects to screen space
4. Pixel shader outputs solid color

#### Gaussian Splat Mode
1. Load full splat attributes
2. **Sort splats by depth** (back-to-front for alpha blending)
3. **Vertex Shader**: Convert scale/opacity from log/logit space, evaluate SH
4. **Geometry Shader**: Expand point to quad billboard, project 3D Gaussian covariance to 2D
5. **Pixel Shader**: Evaluate 2D Gaussian, apply alpha blending

### Stereo Rendering

For Leia SR displays:
1. Render scene twice (left eye, right eye)
2. Side-by-side stereo texture (2× width)
3. SR weaver performs interlacing for autostereoscopic display

---

## Project Structure

```
LeiaSplatViewer/
├── src/
│   ├── main.cpp              - Application entry, DirectX/SR init, main loop
│   ├── Math.h                - Vector/matrix/quaternion utilities
│   ├── SplatLoader.h/cpp     - PLY file parser
│   └── OrbitCamera.h/cpp     - Camera system
├── shaders/
│   ├── PointCloud_VS.hlsl    - Point cloud vertex shader
│   ├── PointCloud_PS.hlsl    - Point cloud pixel shader
│   ├── Splat_VS.hlsl         - Splat vertex shader
│   ├── Splat_GS.hlsl         - Splat geometry shader (billboard expansion)
│   ├── Splat_PS.hlsl         - Splat pixel shader (Gaussian evaluation)
│   └── SplatSort_CS.hlsl     - Depth sorting compute shader
├── LeiaSplatViewer.vcxproj   - Visual Studio project
└── README.md                 - This file
```

---

## Performance

### Targets

- **100K splats**: 60 FPS
- **500K splats**: 30+ FPS
- **1M splats**: 24+ FPS (with GPU sorting)

### Optimizations

- GPU-accelerated depth sorting (compute shader)
- Frustum culling (planned)
- LOD system for distant splats (planned)
- Instanced rendering for quad billboards

---

## Known Limitations

1. **File Format**: Only binary little-endian `.ply` format supported
2. **Sorting**: Per-frame CPU sorting limits scalability (GPU sorting in development)
3. **SH Bands**: Currently only DC component used (full SH evaluation planned)
4. **Single Scene**: No multi-scene loading or switching

---

## Troubleshooting

### Application fails to start

**Cause**: Leia SR Platform Service not running

**Solution**:
1. Open Windows Services (`services.msc`)
2. Find "Leia SR Platform Service"
3. Right-click → Start
4. Wait 30 seconds, then try again

### PLY file fails to load

**Cause**: Unsupported format or corrupt file

**Solution**:
- Verify file is binary little-endian format
- Check header contains required properties (x, y, z, scale, rotation, opacity)
- Try a different PLY file

### Black screen or no 3D effect

**Cause**: Running on wrong monitor or display not in 3D mode

**Solution**:
- Move window to Leia SR display
- Ensure display is in native resolution
- Verify SR Platform Service is running

### Low FPS

**Solution**:
- Build Release configuration (not Debug)
- Close other GPU-intensive applications
- Reduce splat count (try smaller scene)

---

## Building from Source

### Prerequisites

1. **Visual Studio 2019 or later**
   - Workload: "Desktop development with C++"
   - Windows 10 SDK

2. **Leia SR SDK**
   - Extract to: `../simulatedreality-SDK-1.34.8.31132-win64-Release/`
   - Relative to project directory

3. **Leia SR Platform Service**
   - Installed and running

### Build Steps

1. Open `LeiaSplatViewer.vcxproj`
2. Select Platform: **x64** (NOT Win32)
3. Select Configuration: **Release**
4. Build → Build Solution (F7)
5. Output: `bin/x64/Release/LeiaSplatViewer.exe`

### Running from Visual Studio

- Press **Ctrl+F5** (run without debugging)
- Shaders will be found automatically (working directory = project root)

### Running from Windows Explorer

1. Navigate to `bin/x64/Release/`
2. Double-click `LeiaSplatViewer.exe`

⚠️ **Note**: Shader files must be accessible from executable's working directory

---

## Sample Data

### Where to Find Gaussian Splat Scenes

- **Luma AI**: https://lumalabs.ai/
- **Polycam**: https://poly.cam/
- **Gaussian Splatting GitHub**: https://github.com/graphdeco-inria/gaussian-splatting

### Recommended Test Scenes

- Start with small scenes (< 100K splats) for testing
- Medium scenes (100K-500K) for demo
- Large scenes (> 500K) for stress testing

---

## Development

### Adding Full Spherical Harmonics

Currently only DC component is used. To add full SH evaluation:

1. Update `Splat_VS.hlsl` to evaluate all SH bands
2. Pass SH coefficients via structured buffer
3. Implement SH evaluation function (see reference implementation)

### Implementing GPU Sorting

1. Use `SplatSort_CS.hlsl` compute shader
2. Dispatch sort before rendering
3. Use sorted indices for draw call

### Adding HUD Overlay

Reuse HUD class from LookAroundEyesApp:
- Display FPS
- Show splat count
- Show current render mode
- Show camera parameters

---

## References

- **3D Gaussian Splatting Paper**: "3D Gaussian Splatting for Real-Time Radiance Field Rendering" (Kerbl et al., 2023)
- **Splatapult**: https://github.com/hyperlogic/splatapult - Reference C++/OpenGL implementation
- **PLY Format**: http://paulbourke.net/dataformats/ply/
- **Leia SR SDK**: Documentation included with SDK

---

## License

This project uses the Leia SR SDK which is subject to Leia Inc. licensing terms.

---

## Version History

- **v1.0** (2025-01-21): Initial release
  - Point cloud rendering
  - Orbit camera controls
  - PLY file loading
  - Basic splat rendering (simplified)
  - Leia SR integration

---

## Contact

For issues related to:
- **Leia SR SDK**: Leia Developer Portal (https://developer.leia3d.com/)
- **Application bugs**: Create an issue in the project repository

---

**Enjoy exploring Gaussian Splat scenes in 3D on your Leia SR display!**
