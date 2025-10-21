/*!
 * LookAround Eyes Comparison App
 * Dual eye tracking system - switches between Weaver and EyePairListener methods
 */

#pragma once

#include "Math.h"

// Pragma warning isolation for SR SDK headers (SRHydra pattern)
#pragma warning(push)
#pragma warning(disable : 4201)  // nonstandard extension: nameless struct/union
#pragma warning(disable : 5208)  // unnamed class used in typedef name cannot declare members
#pragma warning(disable : 4505)  // unreferenced local function has been removed
#include "sr/sense/eyetracker/eyetracker.h"
#include "sr/sense/eyetracker/eyepairlistener.h"
#include "sr/sense/core/inputstream.h"
#include "sr/weaver/dx11weaver.h"
#pragma warning(pop)

// EyePairListener implementation for callback-based eye tracking
class SREyePairListener : public SR::EyePairListener
{
public:
    SR::InputStream<SR::EyePairStream> stream;
    vec3f left;
    vec3f right;

    SREyePairListener(SR::EyeTracker* tracker);
    virtual void accept(const SR_eyePair& eyePair) override;
};

// Manages both eye tracking methods and allows runtime switching
class DualEyeTracker
{
public:
    enum class Mode {
        Weaver,      // Using weaver->getPredictedEyePositions()
        Listener     // Using EyePairListener callbacks
    };

    DualEyeTracker(SR::SRContext* context, SR::IDX11Weaver1* weaver);
    ~DualEyeTracker();

    // Set the active eye tracking method
    void setMode(Mode mode);

    // Get the current eye tracking method
    Mode getMode() const { return m_currentMode; }

    // Get eye positions using the currently active method
    void getEyePositions(vec3f& leftEye, vec3f& rightEye);

    // Get all eye positions from both methods (for comparison/delta display)
    void getAllEyePositions(vec3f& weaverLeft, vec3f& weaverRight,
                           vec3f& listenerLeft, vec3f& listenerRight);

private:
    Mode m_currentMode;
    SR::IDX11Weaver1* m_weaver;
    SR::EyeTracker* m_eyeTracker;
    SREyePairListener* m_listener;
};
