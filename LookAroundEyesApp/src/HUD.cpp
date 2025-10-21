/*!
 * LookAround Eyes Comparison App
 * HUD implementation
 */

#include "HUD.h"
#include <sstream>
#include <iomanip>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

HUD::HUD()
    : m_d2dFactory(nullptr)
    , m_d2dRenderTarget(nullptr)
    , m_writeFactory(nullptr)
    , m_textFormat(nullptr)
    , m_whiteBrush(nullptr)
    , m_greenBrush(nullptr)
    , m_blackBrush(nullptr)
    , m_redBrush(nullptr)
    , m_yellowBrush(nullptr)
    , m_fpsAccumulator(0.0f)
    , m_frameCount(0)
    , m_currentFPS(0.0f)
    , m_fpsUpdateInterval(0.25f)
{
}

HUD::~HUD()
{
    shutdown();
}

void HUD::initialize(ID3D11Device* d3dDevice, IDXGISwapChain* swapChain)
{
    // Create Direct2D factory
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_d2dFactory);

    // Get backbuffer from swap chain
    IDXGISurface* dxgiSurface = nullptr;
    swapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&dxgiSurface);

    // Create Direct2D render target
    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );

    m_d2dFactory->CreateDxgiSurfaceRenderTarget(dxgiSurface, &rtProps, &m_d2dRenderTarget);
    dxgiSurface->Release();

    // Create DirectWrite factory
    DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        (IUnknown**)&m_writeFactory
    );

    // Create text format
    m_writeFactory->CreateTextFormat(
        L"Arial",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        20.0f,
        L"en-us",
        &m_textFormat
    );

    // Create brushes
    m_d2dRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_whiteBrush);
    m_d2dRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.2f, 1.0f, 0.2f), &m_greenBrush);
    m_d2dRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.7f), &m_blackBrush);
    m_d2dRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.0f, 0.0f), &m_redBrush);
    m_d2dRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 0.0f), &m_yellowBrush);
}

void HUD::updateFPS(float deltaTime)
{
    m_fpsAccumulator += deltaTime;
    m_frameCount++;

    if (m_fpsAccumulator >= m_fpsUpdateInterval)
    {
        m_currentFPS = (float)m_frameCount / m_fpsAccumulator;
        m_fpsAccumulator = 0.0f;
        m_frameCount = 0;
    }
}

void HUD::render(DualEyeTracker::Mode mode, float fps,
                 const vec3f& weaverLeft, const vec3f& weaverRight,
                 const vec3f& listenerLeft, const vec3f& listenerRight)
{
    if (!m_d2dRenderTarget)
        return;

    m_d2dRenderTarget->BeginDraw();

    // Draw semi-transparent background (expanded for delta display)
    D2D1_RECT_F bgRect = D2D1::RectF(10.0f, 10.0f, 600.0f, 200.0f);
    m_d2dRenderTarget->FillRectangle(bgRect, m_blackBrush);

    // Draw mode text
    std::wstring modeText = L"Eye Tracking: ";
    if (mode == DualEyeTracker::Mode::Weaver)
    {
        modeText += L"[WEAVER METHOD]";
    }
    else
    {
        modeText += L"[LISTENER METHOD]";
    }

    D2D1_RECT_F modeRect = D2D1::RectF(20.0f, 20.0f, 580.0f, 50.0f);
    m_d2dRenderTarget->DrawText(
        modeText.c_str(),
        (UINT)modeText.length(),
        m_textFormat,
        modeRect,
        m_greenBrush
    );

    // Draw FPS text
    std::wstringstream fpsStream;
    fpsStream << L"FPS: " << std::fixed << std::setprecision(1) << m_currentFPS;
    std::wstring fpsText = fpsStream.str();

    D2D1_RECT_F fpsRect = D2D1::RectF(20.0f, 55.0f, 580.0f, 85.0f);
    m_d2dRenderTarget->DrawText(
        fpsText.c_str(),
        (UINT)fpsText.length(),
        m_textFormat,
        fpsRect,
        m_whiteBrush
    );

    // Calculate eye position deltas
    vec3f deltaLeft = weaverLeft - listenerLeft;
    vec3f deltaRight = weaverRight - listenerRight;

    float deltaLeftMag = sqrtf(deltaLeft.x * deltaLeft.x +
                                deltaLeft.y * deltaLeft.y +
                                deltaLeft.z * deltaLeft.z);
    float deltaRightMag = sqrtf(deltaRight.x * deltaRight.x +
                                 deltaRight.y * deltaRight.y +
                                 deltaRight.z * deltaRight.z);

    // Determine color based on delta magnitude
    ID2D1SolidColorBrush* deltaLeftBrush = m_greenBrush;
    ID2D1SolidColorBrush* deltaRightBrush = m_greenBrush;

    if (deltaLeftMag > 5.0f)
        deltaLeftBrush = m_redBrush;
    else if (deltaLeftMag > 1.0f)
        deltaLeftBrush = m_yellowBrush;

    if (deltaRightMag > 5.0f)
        deltaRightBrush = m_redBrush;
    else if (deltaRightMag > 1.0f)
        deltaRightBrush = m_yellowBrush;

    // Draw delta text - Left Eye
    std::wstringstream deltaLeftStream;
    deltaLeftStream << L"Left Eye Delta: " << std::fixed << std::setprecision(2)
                    << deltaLeftMag << L" mm";
    std::wstring deltaLeftText = deltaLeftStream.str();

    D2D1_RECT_F deltaLeftRect = D2D1::RectF(20.0f, 90.0f, 580.0f, 120.0f);
    m_d2dRenderTarget->DrawText(
        deltaLeftText.c_str(),
        (UINT)deltaLeftText.length(),
        m_textFormat,
        deltaLeftRect,
        deltaLeftBrush
    );

    // Draw delta text - Right Eye
    std::wstringstream deltaRightStream;
    deltaRightStream << L"Right Eye Delta: " << std::fixed << std::setprecision(2)
                     << deltaRightMag << L" mm";
    std::wstring deltaRightText = deltaRightStream.str();

    D2D1_RECT_F deltaRightRect = D2D1::RectF(20.0f, 125.0f, 580.0f, 155.0f);
    m_d2dRenderTarget->DrawText(
        deltaRightText.c_str(),
        (UINT)deltaRightText.length(),
        m_textFormat,
        deltaRightRect,
        deltaRightBrush
    );

    // Draw status indicator
    float maxDelta = (deltaLeftMag > deltaRightMag) ? deltaLeftMag : deltaRightMag;
    std::wstring statusText;
    ID2D1SolidColorBrush* statusBrush = m_greenBrush;

    if (maxDelta < 1.0f)
    {
        statusText = L"Status: OK (delta < 1mm)";
        statusBrush = m_greenBrush;
    }
    else if (maxDelta < 5.0f)
    {
        statusText = L"Status: WARNING (delta 1-5mm)";
        statusBrush = m_yellowBrush;
    }
    else
    {
        statusText = L"Status: ERROR (delta > 5mm)";
        statusBrush = m_redBrush;
    }

    D2D1_RECT_F statusRect = D2D1::RectF(20.0f, 160.0f, 580.0f, 190.0f);
    m_d2dRenderTarget->DrawText(
        statusText.c_str(),
        (UINT)statusText.length(),
        m_textFormat,
        statusRect,
        statusBrush
    );

    m_d2dRenderTarget->EndDraw();
}

void HUD::shutdown()
{
    if (m_yellowBrush) {
        m_yellowBrush->Release();
        m_yellowBrush = nullptr;
    }
    if (m_redBrush) {
        m_redBrush->Release();
        m_redBrush = nullptr;
    }
    if (m_blackBrush) {
        m_blackBrush->Release();
        m_blackBrush = nullptr;
    }
    if (m_greenBrush) {
        m_greenBrush->Release();
        m_greenBrush = nullptr;
    }
    if (m_whiteBrush) {
        m_whiteBrush->Release();
        m_whiteBrush = nullptr;
    }
    if (m_textFormat) {
        m_textFormat->Release();
        m_textFormat = nullptr;
    }
    if (m_writeFactory) {
        m_writeFactory->Release();
        m_writeFactory = nullptr;
    }
    if (m_d2dRenderTarget) {
        m_d2dRenderTarget->Release();
        m_d2dRenderTarget = nullptr;
    }
    if (m_d2dFactory) {
        m_d2dFactory->Release();
        m_d2dFactory = nullptr;
    }
}
