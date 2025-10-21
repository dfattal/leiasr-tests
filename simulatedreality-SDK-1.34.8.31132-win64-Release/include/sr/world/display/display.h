/*!
 * Copyright (C) 2025 Leia, Inc.
 */

#pragma once

#include "sr/world/worldobject.h"
#include "sr/management/srcontext.h"
#include "sr/utility/commoninterfaces.h"

#include <fstream>

#ifdef WIN32
#   ifdef COMPILING_DLL_SimulatedRealityDisplays
#     define DIMENCOSR_API __declspec(dllexport)
#   else
#     define DIMENCOSR_API __declspec(dllimport)
#   endif
#else
#   define DIMENCOSR_API
#endif

namespace SR {

    /*!
     * \brief Class of WorldObject representing the SR display in real space
     *
     * \ingroup UpstreamInterface API
     * The origin of both worlds is at the center of the display
     */
    class DIMENCOSR_API Display : public WorldObject {
    public:
        /**
         * \brief Creates an instance of a Display class
         *
         * \param context is the environment in which created senses are kept track of
         *
         * \return A pointer to the Display instance
         */
        static Display* create(SRContext& context);

        /**
         * \brief Destructor of the Display instance
         *
         * Explicitly defined because Impl is an incomplete type when this header is evaluated.
         * The destructor is implemented in the implementation file because the Impl class is fully defined there.
         */
        virtual ~Display() = default;

        /**
         * \brief Get the vertical resolution that is accepted by the display
         *
         * \return The vertical resolution that is accepted by the display
         */
        virtual const int getResolutionHeight() = 0;

        /**
         * \brief Get the horizontal resolution that is accepted by the display
         *
         * \return The horizontal resolution that is accepted by the display
         */
        virtual const int getResolutionWidth() = 0;

        /**
         * \brief Get the vertical native resolution of the physical display itself
         *
         * \return The vertical native resolution of the display as an integer
         */
        virtual const int getPhysicalResolutionHeight() = 0;

        /**
         * \brief Get the horizontal native resolution of the physical display itself
         *
         * \return The horizontal native resolution of the display as an integer
         */
        virtual const int getPhysicalResolutionWidth() = 0;

        /**
         * \brief Get the physical height of the display
         *
         * \return The physical height of the display as a float number in cm
         */
        virtual const float getPhysicalSizeHeight() = 0;

        /**
         * \brief Get the physical width of the display
         *
         * \return The physical width of the display as a float number in cm
         */
        virtual const float getPhysicalSizeWidth() = 0;

        /**
         * \brief Get the dot pitch (pixel size)
         *
         * \return The dot pitch (pixel size) as a float number in cm
         */
        virtual const float getDotPitch() = 0;

        /**
        * \brief Finds the SR monitor and return it's location in the display configuration
        *
        * \return A rectangle representing the position and virtual size of the display in the display configuration
        */
        virtual SR_recti getLocation() = 0;

        /**
         * \brief Get the recommended views texture width
         *
         * \return The recommended views texture width (in pixels)
         */
        virtual int getRecommendedViewsTextureWidth() = 0;

        /**
         * \brief Get the recommended views texture height
         *
         * \return The recommended views texture height (in pixels)
         */
        virtual int getRecommendedViewsTextureHeight() = 0;
    };


    class IDisplay : public SR::IQueryInterface
    {
    protected:
        IDisplay() = default;
        virtual ~IDisplay() = 0;

    public:
        /**
         * \brief Returns whether the display parameters are valid
         *
         * \return True if the display parameters are valid, false otherwise (e.g. no SR display found)
         */
        virtual bool isValid() const = 0;

        /**
         * \brief Returns an identifier for the display
         *
         * \return The identifier for the display.
         */
        virtual uint64_t identifier() const = 0;

        /**
         * \brief Refresh the display information to be sure of having the latest configuration
         */
        virtual void refresh() = 0;

        /**
         * \brief Get the vertical resolution that is accepted by the display
         *
         * \return The vertical resolution that is accepted by the display
         */
        virtual int getResolutionHeight() const = 0;

        /**
         * \brief Get the horizontal resolution that is accepted by the display
         *
         * \return The horizontal resolution that is accepted by the display
         */
        virtual int getResolutionWidth() const = 0;

        /**
         * \brief Get the vertical native resolution of the physical display itself
         *
         * \return The vertical native resolution of the display as an integer
         */
        virtual int getPhysicalResolutionHeight() const = 0;

        /**
         * \brief Get the horizontal native resolution of the physical display itself
         *
         * \return The horizontal native resolution of the display as an integer
         */
        virtual int getPhysicalResolutionWidth() const = 0;

        /**
         * \brief Get the physical height of the display
         *
         * \return The physical height of the display as a float number in cm
         */
        virtual float getPhysicalSizeHeight() const = 0;

        /**
         * \brief Get the physical width of the display
         *
         * \return The physical width of the display as a float number in cm
         */
        virtual float getPhysicalSizeWidth() const = 0;

        /**
         * \brief Get the dot pitch (pixel size)
         *
         * \return The dot pitch (pixel size) as a float number in cm
         */
        virtual float getDotPitch() const = 0;

        /**
        * \brief Finds the SR monitor and return it's location in the display configuration
        *
        * \return A rectangle representing the position and virtual size of the display in the display configuration
        */
        virtual SR_recti getLocation() const = 0;

        /**
         * \brief Get the recommended views texture width
         *
         * \return The recommended views texture width (in pixels)
         */
        virtual int getRecommendedViewsTextureWidth() const = 0;

        /**
         * \brief Get the recommended views texture height
         *
         * \return The recommended views texture height (in pixels)
         */
        virtual int getRecommendedViewsTextureHeight() const = 0;

        virtual void getDefaultViewingPosition(float& x_mm, float& y_mm, float& z_mm) const = 0;
    };


    /** \brief Retrieves the main active SR display associated with the given context.
        * The returned object tracks the active SR display in the system.
        *
        * \param context Reference to the SRContext from which to obtain the active display.
        * \return A valid pointer to the IDisplay associated with the specified context.
        */
    extern "C" DIMENCOSR_API SR::IDisplay* GetMainSRDisplay(SR::SRContext& context);
}

#undef DIMENCOSR_API

#if defined(SRDISPLAY_LAZYBINDING) && defined(WIN32)
// Use lazy binding to avoid a hard dependency on the new display interface
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace SR {

/** \brief Retrieves the active display associated with the given context.
 *         This function can be called even if the runtime does not support this interface,
 * \param context Reference to the SRContext from which to obtain the active display.
 * \return A valid pointer to the IDisplay associated with the specified context. nullptr if not supported by the runtime.
 */
inline SR::IDisplay* TryGetMainSRDisplay(SRContext& context)
{
    using FnType = SR::IDisplay * (*)(SRContext& context);

    // static cache of resolved address
    static FnType fn = []() -> FnType {
        HMODULE hMod = GetModuleHandleA("SimulatedRealityDisplays");
        if (!hMod) hMod = GetModuleHandleA("SimulatedRealityDisplays32");
        if (!hMod) return nullptr;
        auto func = reinterpret_cast<FnType>(GetProcAddress(hMod, "GetMainSRDisplay"));
        if (!func) func = reinterpret_cast<FnType>(GetProcAddress(hMod, "_GetMainSRDisplay"));
        return func;
        }();

    return fn ? fn(context) : nullptr;
}
} // SR namespace 
#endif

