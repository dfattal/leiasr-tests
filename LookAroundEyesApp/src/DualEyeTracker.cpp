/*!
 * LookAround Eyes Comparison App
 * Dual eye tracking system implementation
 */

#include <cmath>  // Include standard library headers BEFORE SDK headers to avoid conflicts
#include "DualEyeTracker.h"

// SREyePairListener implementation
SREyePairListener::SREyePairListener(SR::EyeTracker* tracker)
    : left(-30.0f, 0.0f, 600.0f)  // Default left eye position
    , right(30.0f, 0.0f, 600.0f)  // Default right eye position
{
    stream.set(tracker->openEyePairStream(this));
}

void SREyePairListener::accept(const SR_eyePair& eyePair)
{
    // Update eye positions from callback
    left = vec3f((float)eyePair.left.x, (float)eyePair.left.y, (float)eyePair.left.z);
    right = vec3f((float)eyePair.right.x, (float)eyePair.right.y, (float)eyePair.right.z);
}

// DualEyeTracker implementation
DualEyeTracker::DualEyeTracker(SR::SRContext* context, SR::IDX11Weaver1* weaver)
    : m_currentMode(Mode::Weaver)  // Start with weaver method
    , m_weaver(weaver)
    , m_eyeTracker(nullptr)
    , m_listener(nullptr)
{
    // Create eye tracker for listener method
    m_eyeTracker = SR::EyeTracker::create(*context);

    // Create listener (always active for instant switching)
    m_listener = new SREyePairListener(m_eyeTracker);
}

DualEyeTracker::~DualEyeTracker()
{
    // Cleanup in reverse order
    if (m_listener) {
        delete m_listener;
        m_listener = nullptr;
    }

    // Note: m_eyeTracker and m_weaver are managed externally
}

void DualEyeTracker::setMode(Mode mode)
{
    m_currentMode = mode;
}

void DualEyeTracker::getEyePositions(vec3f& leftEye, vec3f& rightEye)
{
    if (m_currentMode == Mode::Weaver)
    {
        // Method 1: Use weaver's predicted eye positions
        float left[3], right[3];
        m_weaver->getPredictedEyePositions(left, right);

        leftEye = vec3f(left[0], left[1], left[2]);
        rightEye = vec3f(right[0], right[1], right[2]);
    }
    else // Mode::Listener
    {
        // Method 2: Use EyePairListener callback data
        leftEye = m_listener->left;
        rightEye = m_listener->right;
    }
}

void DualEyeTracker::getAllEyePositions(vec3f& weaverLeft, vec3f& weaverRight,
                                        vec3f& listenerLeft, vec3f& listenerRight)
{
    // Get weaver positions
    float left[3], right[3];
    m_weaver->getPredictedEyePositions(left, right);
    weaverLeft = vec3f(left[0], left[1], left[2]);
    weaverRight = vec3f(right[0], right[1], right[2]);

    // Get listener positions
    listenerLeft = m_listener->left;
    listenerRight = m_listener->right;
}
