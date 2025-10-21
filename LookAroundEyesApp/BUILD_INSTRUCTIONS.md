# LookAround Eyes Comparison App - Build Instructions

**Platform:** Windows 10/11 (x64)
**IDE:** Visual Studio 2019 or later
**SDK:** Leia SR SDK 1.34.8.31132

---

## Prerequisites

### Required Software
1. **Visual Studio 2019 or later**
   - With "Desktop development with C++" workload
   - Windows 10 SDK (10.0.19041.0 or later)

2. **Leia SR Platform Service**
   - Must be installed and running
   - Download from Leia developer portal

3. **Leia SR Display**
   - Must be connected and configured
   - Display should be detected by Windows

### Required SDK
The project expects the SDK at:
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
└── LookAroundEyesApp/
    ├── src/
    ├── shaders/
    └── LookAroundEyesApp.vcxproj
```

---

## Building the Application

### Option 1: Using Visual Studio GUI

1. **Open the project:**
   ```
   Double-click: LookAroundEyesApp.vcxproj
   ```

2. **Select configuration:**
   - Debug (for development) or Release (for performance)
   - Platform: **x64** (required!)

3. **Build:**
   - Press `F7` or menu: `Build > Build Solution`

4. **Output location:**
   ```
   bin/x64/Debug/LookAroundEyesApp.exe
   OR
   bin/x64/Release/LookAroundEyesApp.exe
   ```

5. **Run:**
   - Press `F5` (with debugging) or `Ctrl+F5` (without debugging)

### Option 2: Using MSBuild Command Line

1. **Open "Developer Command Prompt for VS"**

2. **Navigate to project directory:**
   ```cmd
   cd C:\path\to\leiasr-tests\LookAroundEyesApp
   ```

3. **Build Debug version:**
   ```cmd
   msbuild LookAroundEyesApp.vcxproj /p:Configuration=Debug /p:Platform=x64
   ```

4. **Build Release version:**
   ```cmd
   msbuild LookAroundEyesApp.vcxproj /p:Configuration=Release /p:Platform=x64
   ```

5. **Run:**
   ```cmd
   bin\x64\Debug\LookAroundEyesApp.exe
   ```

---

## Post-Build Automatic Steps

The project is configured to automatically:

1. **Copy SDK DLLs** to output directory:
   - `SimulatedRealityCore.dll`
   - `SimulatedRealityFaceTrackers.dll`
   - `SimulatedRealitySense.dll`
   - `SimulatedRealityDisplays.dll`
   - `SimulatedRealityWeaving.dll`
   - Plus any dependencies

2. **This happens via Post-Build Event:**
   ```cmd
   xcopy /Y /D "..\simulatedreality-SDK-1.34.8.31132-win64-Release\bin\x64\*.dll" "$(OutDir)"
   ```

---

## Running the Application

### Prerequisites for Running

1. **Leia SR Platform Service** must be running
   - Check Windows Services or Task Manager
   - Service name: "Leia SR Platform Service" or similar

2. **Leia display** must be connected and detected
   - Check Windows Display Settings
   - Display should show up as an SR display

3. **Required files in same directory as executable:**
   - `LookAroundEyesApp.exe`
   - All `SimulatedReality*.dll` files (copied automatically)
   - Shaders directory: `shaders/` with `.hlsl` files

### Directory Structure for Running

```
bin/x64/Debug/  (or Release)
├── LookAroundEyesApp.exe
├── SimulatedRealityCore.dll
├── SimulatedRealityFaceTrackers.dll
├── SimulatedRealitySense.dll
├── SimulatedRealityDisplays.dll
├── SimulatedRealityWeaving.dll
└── [other dependencies]

shaders/  (in project root, relative to executable)
├── VertexShader.hlsl
└── PixelShader.hlsl
```

**IMPORTANT:** The shaders must be accessible from the executable's working directory.

### Recommended Setup

**Option A: Run from Visual Studio**
- Shaders will be found automatically (working directory = project root)
- Debug output in console window

**Option B: Copy shaders to output directory**
```cmd
xcopy /Y /S "shaders" "bin\x64\Debug\shaders\"
```
Then run the executable directly from `bin\x64\Debug\`

**Option C: Create a deployment package**
```
LookAroundEyesApp_Portable/
├── LookAroundEyesApp.exe
├── *.dll  (all SR DLLs)
└── shaders/
    ├── VertexShader.hlsl
    └── PixelShader.hlsl
```

---

## Controls

| Key | Action |
|-----|--------|
| **Tab** | Toggle between Weaver and EyePairListener eye tracking methods |
| **F11** | Toggle fullscreen mode |
| **Esc** | Exit application |
| **C** | Simulate context invalidation (for testing) |

---

## Troubleshooting

### Build Errors

**Error: Cannot find SDK headers**
- Check that SDK path is correct in project properties
- Verify path: `$(SolutionDir)..\simulatedreality-SDK-1.34.8.31132-win64-Release\include`

**Error: Unresolved external symbols**
- Verify SDK libraries are in: `$(SolutionDir)..\simulatedreality-SDK-1.34.8.31132-win64-Release\lib\x64`
- Check Platform is set to **x64** (not Win32)

**Error: Cannot open include file 'd3d11.h'**
- Install Windows 10 SDK via Visual Studio Installer
- Workload: "Desktop development with C++"

### Runtime Errors

**"SR service not available"**
- Start the Leia SR Platform Service
- Wait 30 seconds for service to initialize
- Check Windows Services

**"SR display not available"**
- Ensure Leia display is connected
- Check Windows Display Settings
- Try disconnecting/reconnecting the display
- Restart SR Platform Service

**"Failed to create weaver"**
- Display not ready yet (wait longer during initialization)
- Display not in native resolution
- Display duplicated (disable display mirroring)

**Black screen or no 3D effect**
- Verify you're running on the SR display (not primary monitor)
- Check display is not in 2D mode
- Ensure SR Platform Service is running

**Shaders fail to compile**
- Verify `shaders/` directory exists relative to executable
- Check shader files are present: `VertexShader.hlsl`, `PixelShader.hlsl`
- Run from Visual Studio (working directory = project root)
- OR copy shaders to output directory

**Low FPS / Performance issues**
- Build Release configuration (not Debug)
- Close other GPU-intensive applications
- Check GPU usage in Task Manager

---

## Expected Output

When running successfully, you should see:

1. **Console window (Debug build):**
   ```
   LookAround Eyes Comparison App
   ================================
   Waiting for SR service...
   View texture size: 1920x1080
   Physical screen: 600mm x 340mm
   Default viewing position: (0, 0, 650)
   Horizontal FOV: 48.4 degrees
   Vertical FOV: 29.3 degrees
   Weaver created successfully
   Dual eye tracker initialized
   Scene initialized with virtual screen at 400mm
   HUD initialized
   SR context initialized

   Application ready!
   Controls:
     Tab - Switch eye tracking method
     F11 - Toggle fullscreen
     Esc - Exit

   Starting with WEAVER method...
   ```

2. **3D Window showing:**
   - Red cube popping out (near plane, 200mm)
   - Green and blue spheres at screen surface (mid plane, 400mm)
   - Yellow cube receding (far plane, 700mm)
   - All objects rotating slowly
   - HUD overlay showing "WEAVER METHOD" and FPS

3. **When pressing Tab:**
   - Console: `Switched to: LISTENER method`
   - HUD: Shows "LISTENER METHOD"

---

## Performance Expectations

- **Target FPS:** 60 (vsync)
- **Expected FPS:** 60 in both tracking modes
- **Latency:** 2 frames (default weaver setting)
- **Memory usage:** ~200-300 MB

---

## Creating a Standalone Package

To create a portable version for distribution:

1. **Build Release configuration**

2. **Create distribution folder:**
   ```cmd
   mkdir LookAroundEyesApp_v1.0
   cd LookAroundEyesApp_v1.0
   ```

3. **Copy files:**
   ```cmd
   copy ..\bin\x64\Release\LookAroundEyesApp.exe .
   copy ..\bin\x64\Release\*.dll .
   xcopy /S ..\shaders shaders\
   ```

4. **Create README.txt:**
   ```
   LookAround Eyes Comparison App

   Requirements:
   - Leia SR Platform Service installed and running
   - Leia SR display connected

   Usage:
   1. Run LookAroundEyesApp.exe
   2. Press Tab to switch eye tracking methods
   3. Press F11 for fullscreen
   4. Press Esc to exit
   ```

5. **Compress to ZIP** for distribution

---

## Development Tips

### Debugging
- Use Debug configuration for detailed error messages
- Console window shows initialization progress
- Visual Studio debugger can break on DirectX/SR errors

### Modifying the Scene
- Edit `Scene.cpp` to change object positions, colors, sizes
- Adjust `g_virtualScreenDepthMM` in `main.cpp` to change focus plane

### Changing Tracking Latency
- Modify `g_srWeaver->setLatencyInFrames(2);` in `InitializeSR()`
- Try values 1-3 for different latency compensation

### Adding More Objects
- Call `create*PlaneObjects()` in `Scene::initialize()`
- Create new geometry in `Scene.cpp`

---

## Contact & Support

For SDK issues:
- Leia Developer Portal: https://developer.leia3d.com/
- Leia Support

For application issues:
- Check IMPLEMENTATION_STATUS.md for known limitations
- Review PRD (LookAroundEyes-PRD.md) for design details

---

**Version:** 1.0
**Last Updated:** 2025-01-15
