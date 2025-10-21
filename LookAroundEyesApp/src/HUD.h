/*!
 * LookAround Eyes Comparison App
 * HUD overlay - displays current eye tracking mode and FPS
 */

#pragma once

#include "DualEyeTracker.h"
#include <d2d1.h>
#include <dwrite.h>
#include <string>

class HUD
{
public:
    HUD();
    ~HUD();

    // Initialize Direct2D/DirectWrite resources
    void initialize(ID3D11Device* d3dDevice, IDXGISwapChain* swapChain);

    // Render HUD overlay
    void render(DualEyeTracker::Mode mode, float fps,
                const vec3f& weaverLeft, const vec3f& weaverRight,
                const vec3f& listenerLeft, const vec3f& listenerRight);

    // Update FPS (call once per frame with delta time)
    void updateFPS(float deltaTime);

    // Cleanup
    void shutdown();

private:
    // Direct2D/DirectWrite resources
    ID2D1Factory* m_d2dFactory;
    ID2D1RenderTarget* m_d2dRenderTarget;
    IDWriteFactory* m_writeFactory;
    IDWriteTextFormat* m_textFormat;
    ID2D1SolidColorBrush* m_whiteBrush;
    ID2D1SolidColorBrush* m_greenBrush;
    ID2D1SolidColorBrush* m_blackBrush;
    ID2D1SolidColorBrush* m_redBrush;
    ID2D1SolidColorBrush* m_yellowBrush;

    // FPS tracking
    float m_fpsAccumulator;
    int m_frameCount;
    float m_currentFPS;
    float m_fpsUpdateInterval;  // Update every 250ms
};
