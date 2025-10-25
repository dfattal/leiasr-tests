/*!
 * Copyright (C) 2025 Leia, Inc.
 *
 * Modern Display Manager Helper
 * Provides version-safe display access with automatic fallback to legacy Display class
 */

#pragma once

#define SRDISPLAY_LAZYBINDING  // Enable lazy binding for version compatibility
#include "sr/world/display/display.h"
#include <iostream>

namespace SR {
namespace Helper {

/**
 * \brief Wrapper class that provides unified display access with version fallback
 *
 * This class automatically uses IDisplayManager when available (SDK 1.34.8+)
 * and falls back to the legacy Display class for older runtimes.
 */
class DisplayAccess
{
private:
    SRContext& m_context;
    IDisplayManager* m_displayManager;
    IDisplay* m_modernDisplay;
    Display* m_legacyDisplay;
    bool m_usingModernAPI;

public:
    DisplayAccess(SRContext& context)
        : m_context(context)
        , m_displayManager(nullptr)
        , m_modernDisplay(nullptr)
        , m_legacyDisplay(nullptr)
        , m_usingModernAPI(false)
    {
        // Try to get modern DisplayManager first
        m_displayManager = TryGetDisplayManagerInstance(context);
        if (m_displayManager) {
            m_modernDisplay = m_displayManager->getPrimaryActiveSRDisplay();
            m_usingModernAPI = true;
            std::cout << "[DisplayAccess] Using modern IDisplayManager API" << std::endl;
        } else {
            // Fallback to legacy Display
            m_legacyDisplay = Display::create(context);
            m_usingModernAPI = false;
            std::cout << "[DisplayAccess] Falling back to legacy Display API" << std::endl;
        }
    }

    ~DisplayAccess() {
        // Context owns IDisplayManager and IDisplay, no cleanup needed
        // Display is also context-owned, no cleanup needed
    }

    /**
     * \brief Check if display is valid and available
     *
     * For modern API: checks IDisplay::isValid()
     * For legacy API: checks if display parameters are non-zero
     */
    bool isDisplayValid() {
        if (m_usingModernAPI) {
            return m_modernDisplay && m_modernDisplay->isValid();
        } else {
            // Legacy API doesn't have isValid(), check if we got meaningful data
            if (!m_legacyDisplay) return false;
            SR_recti loc = m_legacyDisplay->getLocation();
            int64_t width = loc.right - loc.left;
            int64_t height = loc.bottom - loc.top;
            return (width != 0) && (height != 0);
        }
    }

    /**
     * \brief Get display resolution height
     */
    int getResolutionHeight() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getResolutionHeight() : 1440;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getResolutionHeight() : 1440;
        }
    }

    /**
     * \brief Get display resolution width
     */
    int getResolutionWidth() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getResolutionWidth() : 2560;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getResolutionWidth() : 2560;
        }
    }

    /**
     * \brief Get physical resolution height
     */
    int getPhysicalResolutionHeight() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getPhysicalResolutionHeight() : 1440;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getPhysicalResolutionHeight() : 1440;
        }
    }

    /**
     * \brief Get physical resolution width
     */
    int getPhysicalResolutionWidth() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getPhysicalResolutionWidth() : 2560;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getPhysicalResolutionWidth() : 2560;
        }
    }

    /**
     * \brief Get physical size height in cm
     */
    float getPhysicalSizeHeight() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getPhysicalSizeHeight() : 40.0f;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getPhysicalSizeHeight() : 40.0f;
        }
    }

    /**
     * \brief Get physical size width in cm
     */
    float getPhysicalSizeWidth() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getPhysicalSizeWidth() : 70.0f;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getPhysicalSizeWidth() : 70.0f;
        }
    }

    /**
     * \brief Get dot pitch in cm
     */
    float getDotPitch() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getDotPitch() : 0.0233f;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getDotPitch() : 0.0233f;
        }
    }

    /**
     * \brief Get display location in virtual screen coordinates
     */
    SR_recti getLocation() {
        SR_recti defaultLoc = {0, 0, 2560, 1440};
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getLocation() : defaultLoc;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getLocation() : defaultLoc;
        }
    }

    /**
     * \brief Get recommended views texture width
     */
    int getRecommendedViewsTextureWidth() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getRecommendedViewsTextureWidth() : 2560;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getRecommendedViewsTextureWidth() : 2560;
        }
    }

    /**
     * \brief Get recommended views texture height
     */
    int getRecommendedViewsTextureHeight() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getRecommendedViewsTextureHeight() : 1440;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getRecommendedViewsTextureHeight() : 1440;
        }
    }

    /**
     * \brief Get default viewing position (only available in modern API)
     *
     * \return true if modern API and successfully retrieved, false for legacy API
     */
    bool getDefaultViewingPosition(float& x_mm, float& y_mm, float& z_mm) {
        if (m_usingModernAPI && m_modernDisplay) {
            m_modernDisplay->getDefaultViewingPosition(x_mm, y_mm, z_mm);
            return true;
        } else {
            // Legacy API doesn't support this, provide sensible defaults
            x_mm = 0.0f;
            y_mm = 0.0f;
            z_mm = 600.0f; // Default viewing distance
            return false;
        }
    }

    /**
     * \brief Check if using modern DisplayManager API
     */
    bool isUsingModernAPI() const { return m_usingModernAPI; }

    /**
     * \brief Get unique display identifier (only modern API)
     */
    uint64_t getIdentifier() {
        if (m_usingModernAPI && m_modernDisplay) {
            return m_modernDisplay->identifier();
        }
        return 0; // Legacy API doesn't have identifiers
    }

    /**
     * \brief Wait for display to become ready
     *
     * Polls display validity with timeout
     * \param maxWaitSeconds Maximum time to wait in seconds
     * \return true if display became ready, false if timeout
     */
    bool waitForDisplay(double maxWaitSeconds = 5.0) {
        double startTime = (double)GetTickCount64() / 1000.0;

        while (true) {
            if (isDisplayValid()) {
                std::cout << "[DisplayAccess] Display is ready" << std::endl;
                return true;
            }

            double currentTime = (double)GetTickCount64() / 1000.0;
            if ((currentTime - startTime) > maxWaitSeconds) {
                std::cout << "[DisplayAccess] Display wait timeout" << std::endl;
                return false;
            }

            std::cout << "[DisplayAccess] Waiting for display..." << std::endl;
            Sleep(100);
        }
    }
};

} // namespace Helper
} // namespace SR
