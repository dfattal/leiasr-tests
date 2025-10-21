# How to Run the LookAround Eyes Comparison App

**Quick reference guide for running the application on a Windows machine with Leia SR display.**

---

## System Requirements

✅ **Windows 10 or 11** (64-bit)
✅ **Leia SR Display** connected and configured
✅ **Leia SR Platform Service** installed and running
✅ **Visual Studio 2019 or later** (for building from source)

---

## Option 1: Run Pre-Built Executable (If Available)

If you have a pre-built executable package:

1. **Extract the package** to any folder
2. **Verify files are present:**
   ```
   LookAroundEyesApp.exe
   SimulatedRealityCore.dll
   SimulatedRealityFaceTrackers.dll
   SimulatedRealitySense.dll
   SimulatedRealityDisplays.dll
   SimulatedRealityWeaving.dll
   shaders/VertexShader.hlsl
   shaders/PixelShader.hlsl
   ```

3. **Double-click** `LookAroundEyesApp.exe`

4. **If it doesn't run:**
   - Check that Leia SR Platform Service is running (Windows Services)
   - Ensure Leia display is connected and detected
   - See Troubleshooting section below

---

## Option 2: Build and Run from Source

### Step 1: Prerequisites

1. **Install Visual Studio 2019 or later**
   - Download from: https://visualstudio.microsoft.com/
   - Install workload: "Desktop development with C++"

2. **Verify Leia SR SDK is in correct location**
   ```
   leiasr-tests/
   ├── simulatedreality-SDK-1.34.8.31132-win64-Release/  ← SDK here
   └── LookAroundEyesApp/                                 ← Project here
   ```

3. **Ensure Leia SR Platform Service is running**
   - Open Windows Services (`services.msc`)
   - Find "Leia SR Platform Service" or similar
   - Status should be "Running"
   - If not, right-click and "Start"

### Step 2: Build the Application

**Using Visual Studio GUI:**

1. Navigate to:
   ```
   C:\path\to\leiasr-tests\LookAroundEyesApp\
   ```

2. Double-click:
   ```
   LookAroundEyesApp.vcxproj
   ```

3. In Visual Studio:
   - **Select Platform:** x64 (top toolbar, NOT Win32)
   - **Select Configuration:** Release (or Debug for development)

4. Press **F7** or menu: **Build > Build Solution**

5. Wait for build to complete (should see "Build succeeded")

**Using Command Line:**

1. Open **"Developer Command Prompt for VS 2019"** (or later)

2. Navigate to project:
   ```cmd
   cd C:\path\to\leiasr-tests\LookAroundEyesApp
   ```

3. Build:
   ```cmd
   msbuild LookAroundEyesApp.vcxproj /p:Configuration=Release /p:Platform=x64
   ```

### Step 3: Run the Application

**From Visual Studio:**

1. Press **Ctrl+F5** (run without debugging)
   - OR **F5** (run with debugging)

2. Application should launch with console window (Debug) or just 3D window (Release)

**From Command Line:**

1. Navigate to output folder:
   ```cmd
   cd bin\x64\Release
   ```

2. Run:
   ```cmd
   LookAroundEyesApp.exe
   ```

**From Windows Explorer:**

1. Navigate to:
   ```
   C:\path\to\leiasr-tests\LookAroundEyesApp\bin\x64\Release\
   ```

2. Double-click `LookAroundEyesApp.exe`

⚠️ **IMPORTANT:** If running from Windows Explorer, shaders must be in the right location:
- Either run from Visual Studio (recommended)
- OR copy `shaders/` folder to the same directory as the .exe

---

## What You Should See

### During Startup (Debug build console):
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

### In the 3D Window:

1. **Top-left corner HUD:**
   ```
   Eye Tracking: [WEAVER METHOD]
   FPS: 60.0
   ```

2. **3D Objects rotating:**
   - **Red cube** - popping out toward you (near)
   - **Green sphere** (left) - at screen surface (mid)
   - **Blue sphere** (right) - at screen surface (mid)
   - **Yellow cube** - receding behind screen (far)

3. **Press Tab** - HUD changes to `[LISTENER METHOD]`

4. **Objects should maintain 3D depth** in both modes

---

## Controls

| Key | What It Does |
|-----|--------------|
| **Tab** | Switch between WEAVER and LISTENER eye tracking methods |
| **F11** | Toggle fullscreen mode |
| **Esc** | Exit application |
| **C** | Simulate context invalidation (for testing recovery) |

---

## Troubleshooting

### Problem: "Failed to create SR context"

**Cause:** Leia SR Platform Service not running

**Solution:**
1. Open Windows Services (`Win+R` → type `services.msc`)
2. Find "Leia SR Platform Service"
3. Right-click → Start
4. Wait 30 seconds
5. Try running the app again

---

### Problem: "SR display not available"

**Cause:** Display not connected or not detected

**Solution:**
1. Check display is physically connected
2. Open Windows Display Settings
3. Verify Leia display is detected
4. Try disconnecting and reconnecting
5. Restart SR Platform Service

---

### Problem: Shaders fail to compile

**Cause:** Shader files not found

**Solution (easiest):**
- Run from Visual Studio (Ctrl+F5)

**Solution (manual):**
1. Copy entire `shaders/` folder to same directory as `.exe`
2. Verify structure:
   ```
   bin/x64/Release/
   ├── LookAroundEyesApp.exe
   └── shaders/
       ├── VertexShader.hlsl
       └── PixelShader.hlsl
   ```

---

### Problem: Application crashes immediately

**Possible causes:**

1. **Missing DLLs:**
   - Build again (post-build event copies DLLs automatically)
   - Or manually copy from SDK `bin/x64/` to executable directory

2. **Wrong platform:**
   - Verify building for **x64** (not Win32)
   - In Visual Studio: top toolbar should show "x64"

3. **DirectX not available:**
   - Update graphics drivers
   - Ensure DirectX 11 is supported

---

### Problem: Black screen or no 3D effect

**Possible causes:**

1. **Running on wrong monitor:**
   - App must run on Leia SR display
   - Move window to SR display
   - Or use fullscreen (F11)

2. **Display in 2D mode:**
   - Check Leia display settings
   - Ensure 3D mode is enabled

3. **Service not initialized:**
   - Wait longer during startup (up to 30 seconds)
   - Check console for errors

---

### Problem: Low FPS (below 60)

**Solutions:**
- Build **Release** configuration (not Debug)
- Close other GPU-intensive applications
- Check GPU usage in Task Manager
- Verify vsync is enabled

---

## Expected Performance

✅ **60 FPS** in both tracking modes
✅ **Smooth eye position updates**
✅ **Instant mode switching** (no lag when pressing Tab)
✅ **Stable 3D depth** in both modes

---

## Testing the Comparison

### Quick Test Procedure

1. **Start the app** (should default to WEAVER method)

2. **Observe the scene:**
   - Note how objects appear at different depths
   - Move your head left/right and see parallax
   - Red cube should pop out most

3. **Press Tab** to switch to LISTENER method

4. **Compare:**
   - Does depth perception change?
   - Is parallax still accurate?
   - Does tracking feel different?
   - Is there any lag or stuttering?

5. **Press Tab again** to switch back to WEAVER

6. **Rapid switching:**
   - Press Tab multiple times quickly
   - Both modes should remain stable
   - No visual artifacts

### What to Look For

- ✅ Consistent depth in both modes
- ✅ Smooth tracking in both modes
- ✅ No visual glitches during switching
- ✅ 60 FPS maintained

---

## Getting Help

### Check These First
1. ✅ Leia SR Platform Service running?
2. ✅ Leia display connected and detected?
3. ✅ Built for x64 platform?
4. ✅ All DLLs present in executable directory?
5. ✅ Shaders accessible from executable?

### Documentation
- **Detailed build instructions:** [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md)
- **Project overview:** [README.md](README.md)
- **Implementation details:** [../IMPLEMENTATION_STATUS.md](../IMPLEMENTATION_STATUS.md)
- **Design specification:** [../LookAroundEyes-PRD.md](../LookAroundEyes-PRD.md)

### Support
- Leia Developer Portal: https://developer.leia3d.com/
- Leia SDK Documentation
- Check console output for error messages

---

## Next Steps After Running

1. **Test both tracking methods** thoroughly
2. **Document your observations** about differences
3. **Try different viewing positions** to test tracking accuracy
4. **Experiment with rapid mode switching** (Tab key)
5. **Note any performance differences** between modes

---

**Good luck with your eye tracking comparison testing!**
