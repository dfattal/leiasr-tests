/*!
 * Copyright (C) 2025 Leia, Inc.
 * Modernized example with DisplayManager support
 */

#include "sr/types.h"
#include "sr/sense/core/inputstream.h"
#include "sr/management/srcontext.h"
#include "sr/sense/handtracker/handtracker.h"
#define SRDISPLAY_LAZYBINDING  // Enable modern DisplayManager with fallback
#include "sr/world/display/display.h"
#include "../../common/display_helper.h"
#include <iostream>

SR_point3d myPoint{ 0.0, 1.0, 2.0 };

class MyListener : public SR::HandPoseListener {
public:
    SR::InputStream<SR::HandPoseStream> inputStream;
    // Inherited via HandPoseListener
    virtual void accept(const SR_handPose& handpose) override
    {
        std::cout << "Hand palm position: " << handpose.palm.x << ", "
                  << handpose.palm.y << ", " << handpose.palm.z << "\n";
    }
};

int main() {
    SR::SRContext context;

    // Demonstrate modern DisplayManager usage
    std::cout << "=== Display Information ===" << std::endl;
    SR::Helper::DisplayAccess displayAccess(context);

    if (displayAccess.isDisplayValid()) {
        std::cout << "Display resolution: " << displayAccess.getResolutionWidth()
                  << "x" << displayAccess.getResolutionHeight() << std::endl;
        std::cout << "Physical size: " << displayAccess.getPhysicalSizeWidth()
                  << "cm x " << displayAccess.getPhysicalSizeHeight() << "cm" << std::endl;
        std::cout << "Dot pitch: " << displayAccess.getDotPitch() << "cm" << std::endl;

        float x, y, z;
        if (displayAccess.getDefaultViewingPosition(x, y, z)) {
            std::cout << "Default viewing position: (" << x << ", " << y << ", " << z << ") mm" << std::endl;
        }
        std::cout << "Using modern API: " << (displayAccess.isUsingModernAPI() ? "Yes" : "No (legacy fallback)") << std::endl;
    } else {
        std::cout << "No SR display detected" << std::endl;
    }

    std::cout << "\n=== Hand Tracking ===" << std::endl;
    SR::HandTracker* handTracker = SR::HandTracker::create(context);
    MyListener listener;
    listener.inputStream.set(handTracker->openHandPoseStream(&listener));
    context.initialize();

    std::cout << "Hand tracking initialized. Move your hand in front of the sensor." << std::endl;
    std::cout << "Press any key and Enter to exit..." << std::endl;

    char c;
    std::cin >> c;

    return 0;
}
