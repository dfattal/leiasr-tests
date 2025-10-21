# Leia Gaussian Splat Viewer - Build Instructions

**Platform:** Windows 10/11 (x64)
**IDE:** Visual Studio 2019 or later
**SDK:** Leia SR SDK 1.34.8.31132

---

## Prerequisites

### Required Software

1. **Visual Studio 2019 or later**
   - Download from: https://visualstudio.microsoft.com/
   - Install workload: "Desktop development with C++"
   - Include: Windows 10 SDK (10.0.19041.0 or later)

2. **Leia SR Platform Service**
   - Must be installed and running
   - Download from Leia developer portal
   - Verify service is running in Windows Services (`services.msc`)

3. **Leia SR Display**
   - Must be connected and configured
   - Display should be detected by Windows

### Required SDK

The project expects the Leia SR SDK at:
```
../simulatedreality-SDK-1.34.8.31132-win64-Release/
```

Relative to the project directory. The directory structure should be:
```
leiasr-tests/
├── simulatedreality-SDK-1.34.8.31132-win64-Release/
│   ├── bin/x64/          (DLL files)
│   ├── lib/x64/          (LIB files)
│   └── include/sr/       (Header files)
├── LookAroundEyesApp/    (Previous project)
└── LeiaSplatViewer/      (This project)
    ├── src/
    ├── shaders/
    └── LeiaSplatViewer.vcxproj
```

---

## Building the Application

### Option 1: Using Visual Studio GUI

1. **Open the project:**
   ```
   Double-click: LeiaSplatViewer.vcxproj
   ```

2. **Select configuration:**
   - Platform: **x64** (required - NOT Win32!)
   - Configuration: Debug (for development) or Release (for performance)

3. **Build:**
   - Press `F7` or menu: `Build > Build Solution`
   - Wait for build to complete

4. **Output location:**
   ```
   bin/x64/Debug/LeiaSplatViewer.exe
   OR
   bin/x64/Release/LeiaSplatViewer.exe
   ```

5. **Run:**
   - Press `F5` (with debugging) or `Ctrl+F5` (without debugging)

### Option 2: Using MSBuild Command Line

1. **Open "Developer Command Prompt for VS"**

2. **Navigate to project directory:**
   ```cmd
   cd C:\path\to\leiasr-tests\LeiaSplatViewer
   ```

3. **Build Debug version:**
   ```cmd
   msbuild LeiaSplatViewer.vcxproj /p:Configuration=Debug /p:Platform=x64
   ```

4. **Build Release version:**
   ```cmd
   msbuild LeiaSplatViewer.vcxproj /p:Configuration=Release /p:Platform=x64
   ```

5. **Run:**
   ```cmd
   bin\x64\Release\LeiaSplatViewer.exe
   ```

---

## Post-Build Automatic Steps

The project is configured to automatically copy SDK DLLs to the output directory via Post-Build Event:

```cmd
xcopy /Y /D "..\simulatedreality-SDK-1.34.8.31132-win64-Release\bin\x64\*.dll" "$(OutDir)"
```

This copies:
- `SimulatedRealityCore.dll`
- `SimulatedRealityDisplays.dll`
- `SimulatedRealityWeaving.dll`
- Plus any dependencies

---

## Running the Application

### Prerequisites for Running

1. **Leia SR Platform Service** must be running
   - Check Windows Services (`services.msc`)
   - Service name: "Leia SR Platform Service"
   - Status should be "Running"

2. **Leia display** must be connected and detected
   - Check Windows Display Settings
   - Display should appear as an SR display

3. **Required files in working directory:**
   - `LeiaSplatViewer.exe`
   - All `SimulatedReality*.dll` files (copied automatically)
   - Shaders directory: `shaders/` with `.hlsl` files

### Directory Structure for Running

When running from Visual Studio, the working directory is the project root:
```
LeiaSplatViewer/  (working directory)
├── bin/x64/Debug/
│   ├── LeiaSplatViewer.exe
│   ├── SimulatedRealityCore.dll
│   ├── SimulatedRealityDisplays.dll
│   └── SimulatedRealityWeaving.dll
└── shaders/  (relative to working directory)
    ├── PointCloud_VS.hlsl
    ├── PointCloud_PS.hlsl
    ├── Splat_VS.hlsl
    ├── Splat_GS.hlsl
    ├── Splat_PS.hlsl
    └── SplatSort_CS.hlsl
```

### Recommended Setup

**Option A: Run from Visual Studio** (Easiest)
- Shaders are found automatically (working directory = project root)
- Console output visible in Debug configuration
- Press `Ctrl+F5` to run

**Option B: Run from Windows Explorer**
1. Navigate to: `bin\x64\Release\`
2. Double-click `LeiaSplatViewer.exe`
3. ⚠️ **IMPORTANT**: Shaders must be in `..\..\shaders\` relative to executable

**Option C: Create deployment package** (See below)

---

## First Run

1. **Start the application**
2. **Load a PLY file:**
   - Press `O` or use File → Open
   - Select a Gaussian Splat `.ply` file
   - Wait for loading (progress depends on file size)

3. **View the splats:**
   - Right mouse drag to orbit camera
   - Scroll wheel to zoom
   - Press `C` to toggle point cloud / splat rendering

4. **Expected result:**
   - Colored points appear at splat positions
   - Camera orbits smoothly around scene center
   - 3D depth effect visible on Leia SR display

---

## Controls

| Key | Action |
|-----|--------|
| **Right Mouse Drag** | Orbit camera around splat origin |
| **Mouse Wheel** | Zoom in/out |
| **C** | Toggle point cloud / Gaussian splat rendering |
| **R** | Reset camera to default position |
| **O** | Open PLY file |
| **Esc** | Exit application |

---

## Troubleshooting

### Build Errors

**Error: Cannot find SDK headers**
- Verify SDK path in project properties
- Expected: `$(SolutionDir)..\simulatedreality-SDK-1.34.8.31132-win64-Release\include`
- Check that SDK is extracted to correct location

**Error: Unresolved external symbols**
- Verify SDK libraries are present: `lib/x64/SimulatedReality*.lib`
- Check Platform is set to **x64** (NOT Win32)
- Ensure linking against correct libraries in project properties

**Error: Cannot open include file 'd3d11.h'**
- Install Windows 10 SDK via Visual Studio Installer
- Workload: "Desktop development with C++"

### Runtime Errors

**"Failed to initialize Leia SR"**
- Start the Leia SR Platform Service
- Wait 30 seconds for service to initialize
- Check Windows Services (`services.msc`)

**"Failed to compile shaders"**
- Verify `shaders/` directory exists relative to working directory
- Check shader files are present:
  - `PointCloud_VS.hlsl`, `PointCloud_PS.hlsl`
  - `Splat_VS.hlsl`, `Splat_GS.hlsl`, `Splat_PS.hlsl`
- Run from Visual Studio (working directory = project root)

**Application crashes on startup**
- Check for missing DLLs (post-build event should copy them)
- Verify building for **x64** platform
- Update graphics drivers (DirectX 11 required)

**"Failed to load PLY file"**
- Verify file format is binary little-endian
- Check PLY header contains required properties
- Try a different PLY file

**Black screen or no 3D effect**
- Move window to Leia SR display
- Ensure display is in native resolution
- Verify SR Platform Service is running
- Check display is not in 2D mode

**Low FPS**
- Build Release configuration (not Debug)
- Close other GPU-intensive applications
- Try smaller PLY file (< 100K splats for testing)
- Check GPU usage in Task Manager

---

## Creating a Deployment Package

To create a portable version for distribution:

1. **Build Release configuration**
   ```cmd
   msbuild LeiaSplatViewer.vcxproj /p:Configuration=Release /p:Platform=x64
   ```

2. **Create distribution folder:**
   ```cmd
   mkdir LeiaSplatViewer_v1.0
   cd LeiaSplatViewer_v1.0
   ```

3. **Copy files:**
   ```cmd
   copy ..\bin\x64\Release\LeiaSplatViewer.exe .
   copy ..\bin\x64\Release\*.dll .
   xcopy /S ..\shaders shaders\
   ```

4. **Create README.txt:**
   ```
   Leia Gaussian Splat Viewer

   Requirements:
   - Leia SR Platform Service installed and running
   - Leia SR display connected
   - Gaussian Splat .ply file

   Usage:
   1. Run LeiaSplatViewer.exe
   2. Press O to open .ply file
   3. Right drag to orbit, scroll wheel to zoom
   4. Press C to toggle rendering modes
   5. Press Esc to exit

   Controls:
   - Right Mouse Drag: Orbit camera
   - Mouse Wheel: Zoom
   - C: Toggle point cloud / splat
   - R: Reset camera
   - O: Open file
   - Esc: Exit
   ```

5. **Compress to ZIP** for distribution

---

## Performance Targets

| Splat Count | Target FPS | Expected |
|-------------|------------|----------|
| 10K - 100K | 60 | ✅ Smooth |
| 100K - 500K | 60 | ⚠️ May drop |
| 500K - 1M | 30+ | ⚠️ Requires optimization |
| > 1M | 24+ | ❌ May be slow |

### Optimization Tips

- Build Release configuration for 3-5x performance boost
- GPU sorting (compute shader) significantly improves large scenes
- Frustum culling reduces draw calls (implementation pending)
- LOD system for distant splats (implementation pending)

---

## Development Tips

### Debugging

- Use Debug configuration for detailed error messages
- Set breakpoints in Visual Studio
- Check Output window for shader compilation errors
- Use PIX or RenderDoc for GPU debugging

### Modifying Shaders

1. Edit `.hlsl` files in `shaders/` directory
2. Rebuild project (shaders compile at runtime)
3. Test changes immediately (no need to rebuild exe)

### Adding Features

Refer to `splat-prd.md` and `splat-task-list.md` for:
- Full spherical harmonics implementation
- GPU-based sorting
- HUD overlay
- Advanced rendering features

---

## Sample Data

### Where to Get Gaussian Splat Files

- **Luma AI**: https://lumalabs.ai/ - Capture and download splats
- **Polycam**: https://poly.cam/ - Mobile capture app
- **Gaussian Splatting GitHub**: https://github.com/graphdeco-inria/gaussian-splatting - Example scenes

### Recommended Test Files

1. **Small** (< 10K splats): Quick testing
2. **Medium** (100K-500K splats): Demo quality
3. **Large** (> 500K splats): Stress testing

---

## Contact & Support

For SDK issues:
- Leia Developer Portal: https://developer.leia3d.com/
- Leia Support

For application issues:
- Check IMPLEMENTATION_STATUS in `splat-prd.md`
- Review task list in `splat-task-list.md`

---

**Version:** 1.0
**Last Updated:** 2025-01-21
**Platform:** Windows 10/11 x64
