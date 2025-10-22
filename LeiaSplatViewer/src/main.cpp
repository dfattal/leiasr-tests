/*!
 * Leia Splat Viewer
 * Main application - Gaussian Splat viewer for Leia SR displays
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <commdlg.h>

// Leia SR includes
#include "sr/management/srcontext.h"
#include "sr/world/display/display.h"
#include "sr/weaver/dx11weaver.h"
#include "sr/utility/exception.h"

// App includes - safe to include after pragma pop
#include "leia_math.h"
#include "SplatLoader.h"
#include "OrbitCamera.h"

#include <string>
#include <vector>
#include <algorithm>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "comdlg32.lib")

// Global application state
HWND g_hWnd = nullptr;
int g_windowWidth = 1920;
int g_windowHeight = 1080;
bool g_running = true;
bool g_fullscreen = false;

// DirectX resources
ID3D11Device* g_d3dDevice = nullptr;
ID3D11DeviceContext* g_d3dContext = nullptr;
IDXGISwapChain* g_swapChain = nullptr;
ID3D11RenderTargetView* g_backbufferRTV = nullptr;
ID3D11DepthStencilView* g_depthStencilView = nullptr;

// SR resources
SR::SRContext* g_srContext = nullptr;
SR::IDX11Weaver1* g_srWeaver = nullptr;
ID3D11Texture2D* g_viewTexture = nullptr;
ID3D11RenderTargetView* g_viewRTV = nullptr;
ID3D11DepthStencilView* g_viewDSV = nullptr;
UINT g_viewWidth = 0;
UINT g_viewHeight = 0;

// Rendering mode
enum class RenderMode {
    PointCloud,
    Splat
};
RenderMode g_renderMode = RenderMode::PointCloud;

// Scene data
SplatLoader* g_splatLoader = nullptr;
OrbitCamera* g_camera = nullptr;

// GPU resources for splats
ID3D11Buffer* g_splatVertexBuffer = nullptr;
ID3D11Buffer* g_constantBuffer = nullptr;
ID3D11VertexShader* g_pointCloudVS = nullptr;
ID3D11PixelShader* g_pointCloudPS = nullptr;
ID3D11InputLayout* g_pointCloudInputLayout = nullptr;

// Mouse state
bool g_rightMouseDown = false;
int g_lastMouseX = 0;
int g_lastMouseY = 0;

// Constant buffer structure
struct PerFrameConstants
{
    mat4f viewProjection;
};

// Forward declarations
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool InitializeWindow();
bool InitializeDirectX();
bool InitializeSR();
bool CompileShaders();
bool LoadPLYFile(const char* filePath);
void CreateSplatBuffers();
void Render();
void Cleanup();
std::string OpenFileDialog();

// Utility: Compile shader from file
ID3DBlob* CompileShaderFromFile(const char* filePath, const char* entryPoint, const char* target)
{
    DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT hr = D3DCompileFromFile(
        std::wstring(filePath, filePath + strlen(filePath)).c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        target,
        shaderFlags,
        0,
        &shaderBlob,
        &errorBlob
    );

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        return nullptr;
    }

    if (errorBlob) errorBlob->Release();
    return shaderBlob;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Initialize window
    if (!InitializeWindow())
        return 1;

    // Initialize DirectX
    if (!InitializeDirectX())
    {
        MessageBoxA(nullptr, "Failed to initialize DirectX 11", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Initialize SR
    if (!InitializeSR())
    {
        MessageBoxA(nullptr, "Failed to initialize Leia SR. Please ensure SR Platform Service is running.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Compile shaders
    if (!CompileShaders())
    {
        MessageBoxA(nullptr, "Failed to compile shaders", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Create camera
    g_camera = new OrbitCamera();
    g_camera->initialize(1000.0f, 0.0f, 30.0f * (float)M_PI / 180.0f);

    // Create splat loader
    g_splatLoader = new SplatLoader();

    // Prompt user to load PLY file
    std::string plyPath = OpenFileDialog();
    if (!plyPath.empty())
    {
        if (!LoadPLYFile(plyPath.c_str()))
        {
            MessageBoxA(nullptr, "Failed to load PLY file", "Error", MB_OK | MB_ICONERROR);
        }
        else
        {
            CreateSplatBuffers();
        }
    }

    // Main loop
    MSG msg = {};
    while (g_running)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (g_running)
        {
            g_camera->update();
            Render();
        }
    }

    Cleanup();
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
    case WM_DESTROY:
        g_running = false;
        PostQuitMessage(0);
        return 0;

    case WM_RBUTTONDOWN:
        g_rightMouseDown = true;
        g_lastMouseX = LOWORD(lParam);
        g_lastMouseY = HIWORD(lParam);
        SetCapture(hwnd);
        return 0;

    case WM_RBUTTONUP:
        g_rightMouseDown = false;
        ReleaseCapture();
        return 0;

    case WM_MOUSEMOVE:
        if (g_rightMouseDown)
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            int dx = x - g_lastMouseX;
            int dy = y - g_lastMouseY;

            if (g_camera)
                g_camera->onMouseDrag(dx, dy);

            g_lastMouseX = x;
            g_lastMouseY = y;
        }
        return 0;

    case WM_MOUSEWHEEL:
    {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        if (g_camera)
            g_camera->onMouseWheel(delta);
        return 0;
    }

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            g_running = false;
        }
        else if (wParam == 'C')
        {
            // Toggle render mode
            g_renderMode = (g_renderMode == RenderMode::PointCloud) ? RenderMode::Splat : RenderMode::PointCloud;
        }
        else if (wParam == 'R')
        {
            // Reset camera
            if (g_camera)
                g_camera->reset();
        }
        else if (wParam == 'O')
        {
            // Open file
            std::string plyPath = OpenFileDialog();
            if (!plyPath.empty())
            {
                if (LoadPLYFile(plyPath.c_str()))
                {
                    CreateSplatBuffers();
                    char msg[512];
                    sprintf_s(msg, "Loaded %zu splats successfully", g_splatLoader->getSplatCount());
                    MessageBoxA(g_hWnd, msg, "PLY Loaded", MB_OK | MB_ICONINFORMATION);
                }
                else
                {
                    std::string errorMsg = "Failed to load PLY file";
                    if (g_splatLoader && !g_splatLoader->getErrorMessage().empty())
                    {
                        errorMsg += ":\n" + g_splatLoader->getErrorMessage();
                    }
                    MessageBoxA(g_hWnd, errorMsg.c_str(), "Load Error", MB_OK | MB_ICONERROR);
                }
            }
        }
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool InitializeWindow()
{
    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "LeiaSplatViewerClass";

    if (!RegisterClassExA(&wc))
        return false;

    g_hWnd = CreateWindowExA(
        0,
        "LeiaSplatViewerClass",
        "Leia Gaussian Splat Viewer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        g_windowWidth, g_windowHeight,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (!g_hWnd)
        return false;

    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);

    return true;
}

bool InitializeDirectX()
{
    // Create swap chain
    DXGI_SWAP_CHAIN_DESC scDesc = {};
    scDesc.BufferCount = 2;
    scDesc.BufferDesc.Width = g_windowWidth;
    scDesc.BufferDesc.Height = g_windowHeight;
    scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferDesc.RefreshRate.Numerator = 60;
    scDesc.BufferDesc.RefreshRate.Denominator = 1;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.OutputWindow = g_hWnd;
    scDesc.SampleDesc.Count = 1;
    scDesc.SampleDesc.Quality = 0;
    scDesc.Windowed = TRUE;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &scDesc,
        &g_swapChain,
        &g_d3dDevice,
        &featureLevel,
        &g_d3dContext
    );

    if (FAILED(hr))
        return false;

    // Create backbuffer RTV
    ID3D11Texture2D* backbuffer = nullptr;
    g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer);
    g_d3dDevice->CreateRenderTargetView(backbuffer, nullptr, &g_backbufferRTV);
    backbuffer->Release();

    // Create constant buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(PerFrameConstants);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    g_d3dDevice->CreateBuffer(&cbDesc, nullptr, &g_constantBuffer);

    return true;
}

bool InitializeSR()
{
    // Create SR context with exception handling
    try
    {
        g_srContext = SR::SRContext::create();
    }
    catch (SR::ServerNotAvailableException& e)
    {
        MessageBox(NULL, "Leia SR Platform Service not available. Please ensure it's running.", "SR Error", MB_ICONERROR);
        return false;
    }
    catch (std::exception& e)
    {
        MessageBox(NULL, e.what(), "SR Error", MB_ICONERROR);
        return false;
    }

    if (!g_srContext)
        return false;

    SR::IDisplay* display = SR::GetMainSRDisplay(*g_srContext);
    if (!display)
    {
        MessageBox(NULL, "No Leia SR display found", "SR Error", MB_ICONERROR);
        return false;
    }

    g_viewWidth = display->getRecommendedViewsTextureWidth();
    g_viewHeight = display->getRecommendedViewsTextureHeight();

    // Create view texture (side-by-side stereo)
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = g_viewWidth * 2;
    texDesc.Height = g_viewHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    g_d3dDevice->CreateTexture2D(&texDesc, nullptr, &g_viewTexture);
    g_d3dDevice->CreateRenderTargetView(g_viewTexture, nullptr, &g_viewRTV);

    // Create shader resource view for the view texture
    ID3D11ShaderResourceView* viewSRV = nullptr;
    g_d3dDevice->CreateShaderResourceView(g_viewTexture, nullptr, &viewSRV);

    // Create weaver
    WeaverErrorCode result = SR::CreateDX11Weaver(g_srContext, g_d3dContext, g_hWnd, &g_srWeaver);
    if (result != WeaverErrorCode::WeaverSuccess)
        return false;

    // Configure weaver
    g_srWeaver->setInputViewTexture(viewSRV, g_viewWidth, g_viewHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
    g_srWeaver->setContext(g_d3dContext);

    viewSRV->Release();

    return true;
}

bool CompileShaders()
{
    // Compile point cloud shaders
    ID3DBlob* vsBlob = CompileShaderFromFile("shaders/PointCloud_VS.hlsl", "main", "vs_5_0");
    if (!vsBlob) return false;

    g_d3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_pointCloudVS);

    // Create input layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    g_d3dDevice->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_pointCloudInputLayout);
    vsBlob->Release();

    ID3DBlob* psBlob = CompileShaderFromFile("shaders/PointCloud_PS.hlsl", "main", "ps_5_0");
    if (!psBlob) return false;

    g_d3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_pointCloudPS);
    psBlob->Release();

    return true;
}

bool LoadPLYFile(const char* filePath)
{
    return g_splatLoader->load(filePath);
}

void CreateSplatBuffers()
{
    if (!g_splatLoader || !g_splatLoader->isLoaded())
        return;

    // Release old buffer
    if (g_splatVertexBuffer)
    {
        g_splatVertexBuffer->Release();
        g_splatVertexBuffer = nullptr;
    }

    // Create vertex buffer with splat data
    // For point cloud mode, we just need position + color
    struct PointCloudVertex {
        vec3f position;
        vec3f color;
    };

    const auto& splats = g_splatLoader->getSplats();
    std::vector<PointCloudVertex> vertices(splats.size());

    for (size_t i = 0; i < splats.size(); i++)
    {
        vertices[i].position = splats[i].position;
        vertices[i].color = splats[i].color;
    }

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = (UINT)(vertices.size() * sizeof(PointCloudVertex));
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices.data();

    g_d3dDevice->CreateBuffer(&vbDesc, &vbData, &g_splatVertexBuffer);

    // Auto-frame camera to splat bounds
    vec3f minBounds, maxBounds;
    g_splatLoader->getBounds(minBounds, maxBounds);

    vec3f center = (minBounds + maxBounds) * 0.5f;
    vec3f size = maxBounds - minBounds;
    float maxDim = std::max(std::max(size.x, size.y), size.z);

    // Ensure reasonable distance (handle edge cases)
    float distance = std::max(maxDim * 2.0f, 1.0f);

    g_camera->setTarget(center);
    g_camera->setDistance(distance);
}

void Render()
{
    if (!g_d3dContext || !g_splatLoader || !g_splatLoader->isLoaded())
        return;

    // Clear view texture
    const float clearColor[4] = { 0.1f, 0.1f, 0.15f, 1.0f };
    g_d3dContext->ClearRenderTargetView(g_viewRTV, clearColor);

    // Set render target
    g_d3dContext->OMSetRenderTargets(1, &g_viewRTV, nullptr);

    // Render both eyes
    for (int eye = 0; eye < 2; eye++)
    {
        // Set viewport for this eye
        D3D11_VIEWPORT viewport;
        viewport.TopLeftX = (float)(eye * g_viewWidth);
        viewport.TopLeftY = 0.0f;
        viewport.Width = (float)g_viewWidth;
        viewport.Height = (float)g_viewHeight;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        g_d3dContext->RSSetViewports(1, &viewport);

        // Get view-projection matrix
        float aspect = (float)g_viewWidth / (float)g_viewHeight;
        // Use large far plane to handle any scene size
        mat4f viewProj = g_camera->getViewProjectionMatrix(45.0f * (float)M_PI / 180.0f, aspect, 0.01f, 100000.0f);

        // Update constant buffer
        D3D11_MAPPED_SUBRESOURCE mapped;
        g_d3dContext->Map(g_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        PerFrameConstants* constants = (PerFrameConstants*)mapped.pData;
        constants->viewProjection = viewProj;
        g_d3dContext->Unmap(g_constantBuffer, 0);

        // Set shaders and resources
        g_d3dContext->VSSetShader(g_pointCloudVS, nullptr, 0);
        g_d3dContext->PSSetShader(g_pointCloudPS, nullptr, 0);
        g_d3dContext->VSSetConstantBuffers(0, 1, &g_constantBuffer);
        g_d3dContext->IASetInputLayout(g_pointCloudInputLayout);

        // Set vertex buffer
        UINT stride = sizeof(float) * 6; // position + color
        UINT offset = 0;
        g_d3dContext->IASetVertexBuffers(0, 1, &g_splatVertexBuffer, &stride, &offset);
        g_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

        // Draw
        g_d3dContext->Draw((UINT)g_splatLoader->getSplatCount(), 0);
    }

    // Weave to backbuffer
    if (g_srWeaver)
    {
        g_srWeaver->weave();
    }

    // Present
    g_swapChain->Present(1, 0);
}

void Cleanup()
{
    if (g_camera) delete g_camera;
    if (g_splatLoader) delete g_splatLoader;

    if (g_splatVertexBuffer) g_splatVertexBuffer->Release();
    if (g_constantBuffer) g_constantBuffer->Release();
    if (g_pointCloudVS) g_pointCloudVS->Release();
    if (g_pointCloudPS) g_pointCloudPS->Release();
    if (g_pointCloudInputLayout) g_pointCloudInputLayout->Release();

    if (g_viewRTV) g_viewRTV->Release();
    if (g_viewTexture) g_viewTexture->Release();
    if (g_srWeaver) g_srWeaver->destroy();
    if (g_srContext) SR::SRContext::deleteSRContext(g_srContext);

    if (g_backbufferRTV) g_backbufferRTV->Release();
    if (g_swapChain) g_swapChain->Release();
    if (g_d3dContext) g_d3dContext->Release();
    if (g_d3dDevice) g_d3dDevice->Release();
}

std::string OpenFileDialog()
{
    char filename[MAX_PATH] = "";

    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = g_hWnd;
    ofn.lpstrFilter = "PLY Files (*.ply)\0*.ply\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = "ply";

    if (GetOpenFileNameA(&ofn))
    {
        return std::string(filename);
    }

    return "";
}
