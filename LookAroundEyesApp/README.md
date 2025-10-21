# LookAround Eyes Comparison App

A Windows DirectX 11 application for comparing two eye position tracking methods in the Leia SR SDK.

---

## Overview

This application demonstrates and allows real-time comparison between:
1. **Weaver Method** - `weaver->getPredictedEyePositions()`
2. **EyePairListener Method** - Callback-based `SR::EyePairListener`

### Features
- ✅ Runtime switching between tracking methods (Tab key)
- ✅ 3D scene with objects at multiple depth planes
- ✅ Virtual screen positioned at 400mm for optimal mid-plane focus
- ✅ HUD overlay showing current mode and FPS
- ✅ Automatic FOV calculation from display properties
- ✅ Generalized perspective projection (Kooima 2009)

---

## Quick Start

### Prerequisites
- Windows 10/11 (x64)
- Visual Studio 2019 or later
- Leia SR SDK 1.34.8.31132
- Leia SR Platform Service running
- Leia SR display connected

### Build
```cmd
# Option 1: Open in Visual Studio
LookAroundEyesApp.vcxproj

# Option 2: Command line
msbuild LookAroundEyesApp.vcxproj /p:Configuration=Release /p:Platform=x64
```

### Run
```cmd
bin\x64\Release\LookAroundEyesApp.exe
```

**See [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) for detailed setup.**

---

## Controls

| Key | Action |
|-----|--------|
| **Tab** | Switch between Weaver/Listener tracking methods |
| **F11** | Toggle fullscreen |
| **Esc** | Exit application |
| **C** | Simulate context invalidation (testing) |

---

## Scene Layout

### Object Distribution
| Object | Position | Depth from Screen | Visual Effect |
|--------|----------|-------------------|---------------|
| Red Cube | (0, 100, 200mm) | -200mm | Pops out strongly |
| Green Sphere | (-80, 0, 400mm) | 0mm | At virtual screen |
| Blue Sphere | (80, 0, 400mm) | 0mm | At virtual screen |
| Yellow Cube | (0, -100, 700mm) | +300mm | Recedes moderately |

**Virtual screen at 400mm** - Mid-plane objects appear exactly at screen surface.

---

## Architecture

### Core Components
- **main.cpp** - Application entry, DirectX/SR initialization, render loop
- **DualEyeTracker** - Manages both tracking methods, runtime switching
- **Scene** - 3D geometry generation and rendering
- **HUD** - Direct2D overlay (mode indicator, FPS counter)
- **Math.h** - Vector/matrix utilities, view-projection calculation

### Key Technical Details
- Uses `IDisplay->getRecommendedViewsTextureWidth/Height()` for texture size
- Derives FOV from `IDisplay->getPhysicalSize*()` and `getDefaultViewingPosition()`
- Virtual screen at 400mm (not physical display at 0mm)
- Side-by-side stereo rendering (2× width texture)
- Both tracking methods active simultaneously for instant switching

---

## Documentation

- **[LookAroundEyes-PRD.md](../LookAroundEyes-PRD.md)** - Product Requirements Document
- **[BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md)** - Detailed build and run instructions
- **[IMPLEMENTATION_STATUS.md](../IMPLEMENTATION_STATUS.md)** - Implementation progress
- **[task-list.md](../task-list.md)** - Development task breakdown
- **[LookAroundEyes.md](../LookAroundEyes.md)** - Eye tracking methods overview

---

## Project Structure

```
LookAroundEyesApp/
├── src/
│   ├── main.cpp              - Application entry and main loop
│   ├── Math.h                - Vector/matrix utilities
│   ├── DualEyeTracker.h/cpp  - Dual tracking system
│   ├── Scene.h/cpp           - 3D scene rendering
│   └── HUD.h/cpp             - On-screen overlay
├── shaders/
│   ├── VertexShader.hlsl     - Vertex transformation
│   └── PixelShader.hlsl      - Solid color output
├── LookAroundEyesApp.vcxproj - Visual Studio project
├── README.md                 - This file
└── BUILD_INSTRUCTIONS.md     - Build and run guide
```

---

## Comparison Testing

### What to Observe

When switching between modes (Tab key), pay attention to:

1. **Depth Perception** - Do objects maintain correct depth separation?
2. **Parallax Accuracy** - Is the 3D effect consistent?
3. **Tracking Smoothness** - Are eye position updates smooth?
4. **Latency** - Does one method feel more responsive?
5. **3D Sweet Spot** - Is the optimal viewing position similar?

### Performance
- Both modes should maintain **60 FPS** (vsync)
- No visual artifacts during mode switching
- Smooth eye position updates in both modes

---

## Known Limitations

1. **DirectX 11 only** - Windows-specific implementation
2. **Single user** - Eye tracking optimized for one viewer
3. **Fixed scene** - Objects don't respond to user input (by design)
4. **Context recovery** - Manual restart required if SR service crashes

---

## Troubleshooting

### "SR service not available"
- Start Leia SR Platform Service
- Wait 30 seconds for initialization

### "SR display not available"
- Ensure display is connected and detected
- Check Windows Display Settings
- Try reconnecting the display

### Shaders fail to compile
- Run from Visual Studio (shaders in working directory)
- OR copy `shaders/` folder to executable directory

### No 3D effect
- Verify running on SR display (not primary monitor)
- Check display is in native resolution
- Ensure SR service is running

**See [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) for complete troubleshooting guide.**

---

## Development

### Modifying the Scene
Edit `Scene.cpp`:
- Change object positions, colors, sizes
- Add more objects at different depths
- Adjust rotation speeds

### Changing Virtual Screen Depth
Edit `main.cpp`:
```cpp
float g_virtualScreenDepthMM = 400.0f;  // Change this value
```

### Adjusting Latency
Edit `InitializeSR()` in `main.cpp`:
```cpp
g_srWeaver->setLatencyInFrames(2);  // Try 1-3
```

---

## Credits

- **Leia SR SDK** - Leia Inc.
- **Eye Tracking Methods** - As documented in Leia SR SDK 1.34.8
- **Generalized Perspective Projection** - Kooima (2009)

---

## Version

**Version:** 1.0
**Date:** 2025-01-15
**SDK:** Leia SR SDK 1.34.8.31132
**Platform:** Windows 10/11 x64
