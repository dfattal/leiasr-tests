/*!
 * LookAround Eyes Comparison App
 * Main application - window creation, DirectX/SR initialization, render loop
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellscalingapi.h>
#include <d3d11_1.h>
#include <dxgi1_6.h>
#include <chrono>
#include <iostream>

// Leia SR includes (following SDK examples - no pragma warnings)
#define SRDISPLAY_LAZYBINDING
#include "sr/utility/exception.h"
#include "sr/sense/core/inputstream.h"
#include "sr/sense/system/systemsense.h"
#include "sr/world/display/display.h"
#include "sr/weaver/dx11weaver.h"

// App includes - safe to include after pragma pop
#include "Math.h"
#include "DualEyeTracker.h"
#include "Scene.h"
#include "HUD.h"

#pragma comment(lib, "shcore.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// Safe release macro
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if(x != nullptr) { x->Release(); x = nullptr; }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if(x != nullptr) { delete x; x = nullptr; }
#endif

#ifndef SAFE_DESTROY
#define SAFE_DESTROY(x) if(x != nullptr) { x->destroy(); x = nullptr; }
#endif

// Application name
const char* g_windowTitle = "LookAround Eyes Comparison App";
const char* g_windowClass = g_windowTitle;

// Window state
HWND g_hWnd = NULL;
int g_windowWidth = 1280;
int g_windowHeight = 720;
bool g_fullscreen = false;

// DirectX resources
ID3D11Device* g_d3dDevice = nullptr;
ID3D11DeviceContext* g_d3dContext = nullptr;
IDXGISwapChain* g_swapChain = nullptr;
ID3D11RenderTargetView* g_backbufferRTV = nullptr;

// Stereo view texture (side-by-side)
ID3D11Texture2D* g_viewTexture = nullptr;
ID3D11ShaderResourceView* g_viewSRV = nullptr;
ID3D11RenderTargetView* g_viewRTV = nullptr;
ID3D11Texture2D* g_viewDepthTexture = nullptr;
ID3D11DepthStencilView* g_viewDSV = nullptr;
int g_viewWidth = 0;
int g_viewHeight = 0;

// SR resources
SR::SRContext* g_srContext = nullptr;
SR::IDX11Weaver1* g_srWeaver = nullptr;
DualEyeTracker* g_eyeTracker = nullptr;

// SR system event listener
class SRSystemEventListener : public SR::SystemEventListener
{
public:
    SR::InputStream<SR::SystemEventStream> stream;
    bool contextInvalid = false;

    virtual void accept(const SR::SystemEvent& frame) override
    {
        switch (frame.eventType)
        {
        case SR_eventType::ContextInvalid:
            std::cout << "SR Context Invalid - need to recreate!" << std::endl;
            contextInvalid = true;
            break;
        case SR_eventType::UserFound:
            std::cout << "User found" << std::endl;
            break;
        case SR_eventType::UserLost:
            std::cout << "User lost" << std::endl;
            break;
        default:
            break;
        }
    }
};

SRSystemEventListener* g_systemEventListener = nullptr;

// Scene and HUD
Scene* g_scene = nullptr;
HUD* g_hud = nullptr;

// Display parameters
float g_screenWidthMM = 0.0f;
float g_screenHeightMM = 0.0f;
float g_virtualScreenDepthMM = 400.0f;  // Virtual screen at 400mm for mid-plane focus

// Timing
auto g_startTime = std::chrono::high_resolution_clock::now();
float g_lastFrameTime = 0.0f;

// Forward declarations
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
bool InitializeWindow(HINSTANCE hInstance);
bool InitializeDirectX();
bool InitializeSR(double maxWaitTime);
void Render();
void Cleanup();
void SetFullscreen(bool fullscreen);

// Get current time in seconds
float GetTime()
{
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = now - g_startTime;
    return elapsed.count();
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CLOSE:
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            return 0;

        case VK_TAB:
        {
            // Toggle eye tracking mode
            if (g_eyeTracker)
            {
                auto currentMode = g_eyeTracker->getMode();
                auto newMode = (currentMode == DualEyeTracker::Mode::Weaver)
                    ? DualEyeTracker::Mode::Listener
                    : DualEyeTracker::Mode::Weaver;
                g_eyeTracker->setMode(newMode);

                std::cout << "Switched to: "
                    << (newMode == DualEyeTracker::Mode::Weaver ? "WEAVER" : "LISTENER")
                    << " method" << std::endl;
            }
            return 0;
        }

        case VK_F11:
            g_fullscreen = !g_fullscreen;
            SetFullscreen(g_fullscreen);
            return 0;

        case 'C':
        case 'c':
            // Simulate context invalidation for testing
            if (g_systemEventListener)
            {
                g_systemEventListener->contextInvalid = true;
                std::cout << "Simulating context invalidation..." << std::endl;
            }
            return 0;
        }
        break;

    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            g_windowWidth = LOWORD(lParam);
            g_windowHeight = HIWORD(lParam);
        }
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

bool InitializeWindow(HINSTANCE hInstance)
{
    // Register window class
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = g_windowClass;

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL, "Failed to register window class", g_windowTitle, MB_ICONERROR);
        return false;
    }

    // Create window
    RECT windowRect = { 0, 0, g_windowWidth, g_windowHeight };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    g_hWnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        g_windowClass,
        g_windowTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL, NULL, hInstance, NULL
    );

    if (!g_hWnd)
    {
        MessageBox(NULL, "Failed to create window", g_windowTitle, MB_ICONERROR);
        return false;
    }

    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);

    return true;
}

bool InitializeDirectX()
{
    // Create device and context
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL featureLevel;

    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_DEBUG,
        featureLevels,
        1,
        D3D11_SDK_VERSION,
        &g_d3dDevice,
        &featureLevel,
        &g_d3dContext
    );

    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to create D3D11 device", g_windowTitle, MB_ICONERROR);
        return false;
    }

    // Create swap chain
    IDXGIDevice* dxgiDevice = nullptr;
    g_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);

    IDXGIAdapter* dxgiAdapter = nullptr;
    dxgiDevice->GetAdapter(&dxgiAdapter);

    IDXGIFactory* dxgiFactory = nullptr;
    dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = g_windowWidth;
    swapChainDesc.BufferDesc.Height = g_windowHeight;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = g_hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    hr = dxgiFactory->CreateSwapChain(g_d3dDevice, &swapChainDesc, &g_swapChain);

    dxgiFactory->Release();
    dxgiAdapter->Release();
    dxgiDevice->Release();

    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to create swap chain", g_windowTitle, MB_ICONERROR);
        return false;
    }

    // Create backbuffer render target view
    ID3D11Texture2D* backbuffer = nullptr;
    g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer);
    g_d3dDevice->CreateRenderTargetView(backbuffer, nullptr, &g_backbufferRTV);
    backbuffer->Release();

    return true;
}

bool InitializeSR(double maxWaitTime)
{
    const double startTime = (double)GetTickCount64() / 1000.0;

    // Create SR context with retry logic
    while (g_srContext == nullptr)
    {
        try
        {
            g_srContext = SR::SRContext::create();
            break;
        }
        catch (SR::ServerNotAvailableException& e)
        {
            std::cout << "Waiting for SR service..." << std::endl;
            Sleep(100);
        }

        double curTime = (double)GetTickCount64() / 1000.0;
        if ((curTime - startTime) > maxWaitTime)
        {
            MessageBox(NULL, "SR service not available", g_windowTitle, MB_ICONERROR);
            return false;
        }
    }

    // Wait for display to be ready and get display properties
    SR::IDisplay* display = SR::GetMainSRDisplay(*g_srContext);
    if (!display || !display->isValid())
    {
        MessageBox(NULL, "SR display not available", g_windowTitle, MB_ICONERROR);
        return false;
    }

    // Get recommended texture dimensions
    g_viewWidth = display->getRecommendedViewsTextureWidth();
    g_viewHeight = display->getRecommendedViewsTextureHeight();

    std::cout << "View texture size: " << g_viewWidth << "x" << g_viewHeight << std::endl;

    // Get physical screen dimensions (convert cm to mm)
    g_screenWidthMM = display->getPhysicalSizeWidth() * 10.0f;
    g_screenHeightMM = display->getPhysicalSizeHeight() * 10.0f;

    std::cout << "Physical screen: " << g_screenWidthMM << "mm x " << g_screenHeightMM << "mm" << std::endl;

    // Get default viewing position
    float viewPosX, viewPosY, viewPosZ;
    display->getDefaultViewingPosition(viewPosX, viewPosY, viewPosZ);

    std::cout << "Default viewing position: (" << viewPosX << ", " << viewPosY << ", " << viewPosZ << ")" << std::endl;

    // Calculate FOV from viewing pyramid
    float horizontalFOV = 2.0f * atan2f(g_screenWidthMM / 2.0f, viewPosZ);
    float verticalFOV = 2.0f * atan2f(g_screenHeightMM / 2.0f, viewPosZ);

    std::cout << "Horizontal FOV: " << (horizontalFOV * 180.0f / M_PI) << " degrees" << std::endl;
    std::cout << "Vertical FOV: " << (verticalFOV * 180.0f / M_PI) << " degrees" << std::endl;

    // Create stereo view texture (2× width for side-by-side)
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = g_viewWidth * 2;
    texDesc.Height = g_viewHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

    g_d3dDevice->CreateTexture2D(&texDesc, nullptr, &g_viewTexture);
    g_d3dDevice->CreateShaderResourceView(g_viewTexture, nullptr, &g_viewSRV);
    g_d3dDevice->CreateRenderTargetView(g_viewTexture, nullptr, &g_viewRTV);

    // Create depth texture
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = g_viewWidth * 2;
    depthDesc.Height = g_viewHeight;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    g_d3dDevice->CreateTexture2D(&depthDesc, nullptr, &g_viewDepthTexture);
    g_d3dDevice->CreateDepthStencilView(g_viewDepthTexture, nullptr, &g_viewDSV);

    // Create weaver
    WeaverErrorCode result = SR::CreateDX11Weaver(*g_srContext, g_d3dContext, g_hWnd, &g_srWeaver);
    if (result != WeaverErrorCode::WeaverSuccess)
    {
        MessageBox(NULL, "Failed to create weaver", g_windowTitle, MB_ICONERROR);
        return false;
    }

    g_srWeaver->setInputViewTexture(g_viewSRV, g_viewWidth, g_viewHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
    g_srWeaver->setLatencyInFrames(2);
    g_srWeaver->setShaderSRGBConversion(true, true);

    std::cout << "Weaver created successfully" << std::endl;

    // Initialize dual eye tracker
    g_eyeTracker = new DualEyeTracker(g_srContext, g_srWeaver);

    std::cout << "Dual eye tracker initialized" << std::endl;

    // Setup system event listener
    SR::SystemSense* systemSense = SR::SystemSense::create(*g_srContext);
    g_systemEventListener = new SRSystemEventListener();
    g_systemEventListener->stream.set(systemSense->openSystemEventStream(g_systemEventListener));

    // Initialize scene with display parameters
    g_scene = new Scene();
    g_scene->initialize(g_d3dDevice, g_screenWidthMM, g_screenHeightMM, g_virtualScreenDepthMM);

    std::cout << "Scene initialized with virtual screen at " << g_virtualScreenDepthMM << "mm" << std::endl;

    // Initialize HUD
    g_hud = new HUD();
    g_hud->initialize(g_d3dDevice, g_swapChain);

    std::cout << "HUD initialized" << std::endl;

    // Initialize context LAST (critical!)
    g_srContext->initialize();

    std::cout << "SR context initialized" << std::endl;

    return true;
}

void Render()
{
    // Get current time
    float currentTime = GetTime();
    float deltaTime = currentTime - g_lastFrameTime;
    g_lastFrameTime = currentTime;

    // Update HUD FPS
    if (g_hud)
    {
        g_hud->updateFPS(deltaTime);
    }

    // Get eye positions using current tracking mode
    vec3f leftEye, rightEye;
    vec3f weaverLeft, weaverRight, listenerLeft, listenerRight;
    if (g_eyeTracker)
    {
        g_eyeTracker->getEyePositions(leftEye, rightEye);
        // Also get all positions for HUD delta display
        g_eyeTracker->getAllEyePositions(weaverLeft, weaverRight, listenerLeft, listenerRight);
    }

    // Clear stereo view texture
    const float clearColor[4] = { 0.1f, 0.1f, 0.2f, 1.0f };
    g_d3dContext->ClearRenderTargetView(g_viewRTV, clearColor);
    g_d3dContext->ClearDepthStencilView(g_viewDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Set stereo texture as render target
    g_d3dContext->OMSetRenderTargets(1, &g_viewRTV, g_viewDSV);

    // Render both eyes side-by-side
    for (int i = 0; i < 2; i++)
    {
        // Set viewport for this eye
        D3D11_VIEWPORT viewport;
        viewport.TopLeftX = (FLOAT)(i * g_viewWidth);
        viewport.TopLeftY = 0.0f;
        viewport.Width = (FLOAT)g_viewWidth;
        viewport.Height = (FLOAT)g_viewHeight;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        g_d3dContext->RSSetViewports(1, &viewport);

        // Render scene for this eye
        vec3f eyePos = (i == 0) ? leftEye : rightEye;
        if (g_scene)
        {
            g_scene->render(g_d3dContext, eyePos, currentTime);
        }
    }

    // Set backbuffer as render target
    g_d3dContext->OMSetRenderTargets(1, &g_backbufferRTV, nullptr);

    // Reset viewport to full window
    D3D11_VIEWPORT fullViewport;
    fullViewport.TopLeftX = 0.0f;
    fullViewport.TopLeftY = 0.0f;
    fullViewport.Width = (FLOAT)g_windowWidth;
    fullViewport.Height = (FLOAT)g_windowHeight;
    fullViewport.MinDepth = 0.0f;
    fullViewport.MaxDepth = 1.0f;
    g_d3dContext->RSSetViewports(1, &fullViewport);

    // Perform weaving
    if (g_srWeaver)
    {
        g_srWeaver->weave();
    }

    // Render HUD overlay
    if (g_hud && g_eyeTracker)
    {
        g_hud->render(g_eyeTracker->getMode(), 0.0f,
                     weaverLeft, weaverRight, listenerLeft, listenerRight);
    }

    // Present
    g_swapChain->Present(1, 0);  // vsync
}

void SetFullscreen(bool fullscreen)
{
    static RECT s_windowRect = {};

    if (fullscreen)
    {
        // Save current window rect
        GetWindowRect(g_hWnd, &s_windowRect);

        // Get monitor info
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(MonitorFromWindow(g_hWnd, MONITOR_DEFAULTTOPRIMARY), &mi);

        // Set fullscreen
        SetWindowLong(g_hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(g_hWnd, HWND_TOP,
            mi.rcMonitor.left, mi.rcMonitor.top,
            mi.rcMonitor.right - mi.rcMonitor.left,
            mi.rcMonitor.bottom - mi.rcMonitor.top,
            SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    }
    else
    {
        // Restore windowed mode
        SetWindowLong(g_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
        SetWindowPos(g_hWnd, HWND_NOTOPMOST,
            s_windowRect.left, s_windowRect.top,
            s_windowRect.right - s_windowRect.left,
            s_windowRect.bottom - s_windowRect.top,
            SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    }
}

void Cleanup()
{
    // Cleanup in reverse order of creation
    if (g_hud) {
        g_hud->shutdown();
        delete g_hud;
        g_hud = nullptr;
    }

    if (g_scene) {
        g_scene->shutdown();
        delete g_scene;
        g_scene = nullptr;
    }

    SAFE_DELETE(g_systemEventListener);
    SAFE_DELETE(g_eyeTracker);
    SAFE_DESTROY(g_srWeaver);

    if (g_srContext) {
        SR::SRContext::deleteSRContext(g_srContext);
        g_srContext = nullptr;
    }

    SAFE_RELEASE(g_viewDSV);
    SAFE_RELEASE(g_viewDepthTexture);
    SAFE_RELEASE(g_viewRTV);
    SAFE_RELEASE(g_viewSRV);
    SAFE_RELEASE(g_viewTexture);

    SAFE_RELEASE(g_backbufferRTV);
    SAFE_RELEASE(g_swapChain);
    SAFE_RELEASE(g_d3dContext);
    SAFE_RELEASE(g_d3dDevice);
}

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
    // Set DPI awareness
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    // Create console for debug output
#ifdef _DEBUG
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    std::cout << "LookAround Eyes Comparison App" << std::endl;
    std::cout << "================================" << std::endl;
#endif

    // Initialize window
    if (!InitializeWindow(hInstance))
    {
        return 1;
    }

    // Initialize DirectX
    if (!InitializeDirectX())
    {
        Cleanup();
        return 2;
    }

    // Initialize SR
    if (!InitializeSR(30.0))
    {
        Cleanup();
        return 3;
    }

    std::cout << "\nApplication ready!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Tab - Switch eye tracking method" << std::endl;
    std::cout << "  F11 - Toggle fullscreen" << std::endl;
    std::cout << "  Esc - Exit" << std::endl;
    std::cout << "\nStarting with WEAVER method..." << std::endl;

    // Main loop
    bool running = true;
    while (running)
    {
        // Process messages
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!running)
            break;

        // Check for context invalidation
        if (g_systemEventListener && g_systemEventListener->contextInvalid)
        {
            std::cout << "Context invalidated - need manual restart for now" << std::endl;
            // TODO: Implement context recovery
            running = false;
            break;
        }

        // Render
        Render();
    }

    // Cleanup
    std::cout << "\nShutting down..." << std::endl;
    Cleanup();

    return 0;
}
