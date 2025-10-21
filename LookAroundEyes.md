# Eye Position Tracking for Leia SR 3D Displays

## Overview

The Leia SR SDK provides two methods for obtaining the user's eye position for 3D rendering with autostereoscopic displays. Both methods track the user's left and right eye positions in 3D space (in millimeters) to enable proper perspective-correct rendering.

## Method 1: Weaver's getPredictedEyePositions()

### Description
The modern and recommended approach uses the weaver object's `getPredictedEyePositions()` method, which automatically handles eye tracking, prediction, and latency compensation.

### Location
Defined in `IWeaverBase.h:111`

```cpp
virtual void getPredictedEyePositions(float* left, float* right) = 0;
```

### Usage
```cpp
// Get predicted eye positions from weaver
float leftEye[3], rightEye[3];
weaver->getPredictedEyePositions(leftEye, rightEye);

// Use for stereo rendering
for (int i = 0; i < 2; i++) {
    vec3f eyePos = (i == 0) ? leftEye : rightEye;
    mat4f viewProj = CalculateViewProjectionMatrix(eyePos);
    RenderScene(viewProj);
}
```

### Example (from OpenGL example, line 1079)
```cpp
#if !defined(USE_DEPRECATED_WEAVER)
    g_srWeaver->getPredictedEyePositions(&leftEye.x, &rightEye.x);
#endif
```

### Advantages
- **Automatic prediction**: Compensates for rendering pipeline latency
- **Latency control**: Respects `setLatency()` or `setLatencyInFrames()` settings
- **Integrated**: No need to manage separate listener/stream objects
- **Modern API**: Part of the current SDK interface (`IWeaverBase1`)

### Notes
- Returns predicted positions based on the latency set via `setLatencyInFrames()` or `setLatency()`
- Default latency is 2 frames (typical for vsync + driver buffering)
- The positions may not fully compensate for all latencies, but are optimized for the weaving process

## Method 2: EyePairListener

### Description
The callback-based approach uses an `EyePairListener` to receive real-time eye position updates through an event stream. This is the legacy method, typically used with deprecated weaver APIs.

### Location
Defined in `eyepairlistener.h:15-26`

```cpp
class EyePairListener {
public:
    virtual void accept(const SR_eyePair& frame) = 0;
};
```

### Data Structure
The `SR_eyePair` struct (from `eyepair.h:23-33`) contains:
```cpp
typedef struct {
    uint64_t frameId;     // Autoincrement frame number
    uint64_t time;        // Time since epoch in microseconds
    union {
        SR_point3d eyes[2]; // Absolute positions of left and right eye (mm)
        struct {
            SR_point3d left;  // Absolute position of left eye (mm)
            SR_point3d right; // Absolute position of right eye (mm)
        };
    };
} SR_eyePair;
```

### Usage
```cpp
// Step 1: Create custom listener class
class SREyePairListener : public SR::EyePairListener
{
public:
    SR::InputStream<SR::EyePairStream> stream;
    vec3f left = vec3f(-30.0f, 0.0f, 600.0f);   // Default values
    vec3f right = vec3f(30.0f, 0.0f, 600.0f);

    SREyePairListener(SR::EyeTracker* tracker) {
        stream.set(tracker->openEyePairStream(this));
    }

    virtual void accept(const SR_eyePair& eyePair) override {
        left = vec3f((float)eyePair.left.x,
                     (float)eyePair.left.y,
                     (float)eyePair.left.z);
        right = vec3f((float)eyePair.right.x,
                      (float)eyePair.right.y,
                      (float)eyePair.right.z);
    }
};

// Step 2: Create listener instance
SREyePairListener* listener = new SREyePairListener(
    SR::EyeTracker::create(*g_srContext)
);

// Step 3: Use eye positions in rendering
vec3f leftEye = listener->left;
vec3f rightEye = listener->right;
```

### Example (from OpenGL example, lines 80-97)
```cpp
#ifdef USE_DEPRECATED_WEAVER
    leftEye = g_srEyePairListener->left;
    rightEye = g_srEyePairListener->right;
#endif
```

### Advantages
- **Real-time updates**: Frequent callbacks with fresh eye tracking data
- **Direct access**: Raw tracking data without automatic prediction
- **Timestamp info**: Includes frame ID and timestamp for custom prediction

### Disadvantages
- **Manual prediction**: Must implement your own latency compensation
- **Extra objects**: Requires managing EyeTracker and stream lifecycle
- **Deprecated pattern**: Used with older weaver APIs

## Coordinate System

Both methods use the SR SDK coordinate system:

- **Units**: Millimeters
- **Origin**: Center of the display
- **X-axis**: Right positive
- **Y-axis**: Up positive
- **Z-axis**: Backward positive (away from display toward user)

Example positions:
- `left: (-30mm, 0mm, 600mm)` - Left eye, 30mm left of center, 600mm from screen
- `right: (30mm, 0mm, 600mm)` - Right eye, 30mm right of center, 600mm from screen

## Recommendation

**Use Method 1 (weaver->getPredictedEyePositions())** for new development:
- Simpler API
- Automatic latency compensation
- Integrated with modern weaver interface
- No extra objects to manage

**Use Method 2 (EyePairListener)** only when:
- Working with legacy code using deprecated weavers
- Need access to raw tracking data with timestamps
- Implementing custom prediction algorithms

## Related Concepts

### PredictingWeaverTracker
The `PredictingWeaverTracker` class (`predictingweavertracker.h`) combines both approaches - it implements `EyePairListener` to receive raw eye tracking data and provides predicted positions for weaving. This is the internal mechanism used by modern weavers.

### Latency Control
Set rendering pipeline latency for proper prediction:
```cpp
// In frames (recommended)
weaver->setLatencyInFrames(2);  // Default for vsync

// Or in microseconds
weaver->setLatency(33333);  // 2 frames at 60 FPS
```

### Late Latching
Enable for even lower latency:
```cpp
weaver->enableLateLatching(true);
```
This updates eye positions for frames already in flight, but requires calling `weave()` once per frame.
