/*!
 * LookAround Eyes Comparison App
 * Scene implementation - geometry generation and rendering
 */

#include "Scene.h"
#include <d3dcompiler.h>
#include <cmath>

#pragma comment(lib, "d3dcompiler.lib")

Scene::Scene()
    : m_screenWidthMM(0.0f)
    , m_screenHeightMM(0.0f)
    , m_virtualScreenDepthMM(400.0f)
    , m_constantBuffer(nullptr)
    , m_vertexShader(nullptr)
    , m_pixelShader(nullptr)
    , m_inputLayout(nullptr)
{
}

Scene::~Scene()
{
    shutdown();
}

void Scene::initialize(ID3D11Device* device,
                       float screenWidthMM,
                       float screenHeightMM,
                       float virtualScreenDepthMM)
{
    m_screenWidthMM = screenWidthMM;
    m_screenHeightMM = screenHeightMM;
    m_virtualScreenDepthMM = virtualScreenDepthMM;

    // Compile shaders
    compileShaders(device);

    // Create constant buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(SceneConstants);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer);

    // Create scene objects
    createNearPlaneObjects(device);
    createMidPlaneObjects(device);
    createFarPlaneObjects(device);
}

void Scene::createNearPlaneObjects(ID3D11Device* device)
{
    // Red cube at 200mm depth (pops out 200mm from virtual screen)
    SceneObject redCube;
    redCube.position = vec3f(0.0f, 100.0f, 200.0f);
    redCube.color = vec3f(1.0f, 0.2f, 0.2f);  // Red
    redCube.size = 40.0f;
    redCube.rotationSpeed = 0.5f;  // radians/second
    redCube.type = ObjectType::Cube;

    createCubeGeometry(device, redCube);
    m_objects.push_back(redCube);
}

void Scene::createMidPlaneObjects(ID3D11Device* device)
{
    // Green sphere (left) at 400mm depth (at virtual screen surface)
    SceneObject greenSphere;
    greenSphere.position = vec3f(-80.0f, 0.0f, 400.0f);
    greenSphere.color = vec3f(0.2f, 1.0f, 0.2f);  // Green
    greenSphere.size = 30.0f;
    greenSphere.rotationSpeed = 0.3f;
    greenSphere.type = ObjectType::Sphere;

    createSphereGeometry(device, greenSphere);
    m_objects.push_back(greenSphere);

    // Blue sphere (right) at 400mm depth (at virtual screen surface)
    SceneObject blueSphere;
    blueSphere.position = vec3f(80.0f, 0.0f, 400.0f);
    blueSphere.color = vec3f(0.2f, 0.2f, 1.0f);  // Blue
    blueSphere.size = 30.0f;
    blueSphere.rotationSpeed = 0.3f;
    blueSphere.type = ObjectType::Sphere;

    createSphereGeometry(device, blueSphere);
    m_objects.push_back(blueSphere);
}

void Scene::createFarPlaneObjects(ID3D11Device* device)
{
    // Yellow cube at 700mm depth (recedes 300mm behind virtual screen)
    SceneObject yellowCube;
    yellowCube.position = vec3f(0.0f, -100.0f, 700.0f);
    yellowCube.color = vec3f(1.0f, 1.0f, 0.2f);  // Yellow
    yellowCube.size = 60.0f;
    yellowCube.rotationSpeed = 0.2f;
    yellowCube.type = ObjectType::Cube;

    createCubeGeometry(device, yellowCube);
    m_objects.push_back(yellowCube);
}

void Scene::createCubeGeometry(ID3D11Device* device, SceneObject& obj)
{
    const float s = obj.size / 2.0f;  // Half-size

    // Define cube vertices (8 corners)
    Vertex vertices[] = {
        // Front face
        {{-s, -s, -s}, {obj.color.x, obj.color.y, obj.color.z}},
        {{ s, -s, -s}, {obj.color.x, obj.color.y, obj.color.z}},
        {{ s,  s, -s}, {obj.color.x, obj.color.y, obj.color.z}},
        {{-s,  s, -s}, {obj.color.x, obj.color.y, obj.color.z}},
        // Back face
        {{-s, -s,  s}, {obj.color.x, obj.color.y, obj.color.z}},
        {{ s, -s,  s}, {obj.color.x, obj.color.y, obj.color.z}},
        {{ s,  s,  s}, {obj.color.x, obj.color.y, obj.color.z}},
        {{-s,  s,  s}, {obj.color.x, obj.color.y, obj.color.z}}
    };

    // Define cube indices (36 indices for 12 triangles)
    UINT indices[] = {
        // Front
        0, 1, 2,  0, 2, 3,
        // Right
        1, 5, 6,  1, 6, 2,
        // Back
        5, 4, 7,  5, 7, 6,
        // Left
        4, 0, 3,  4, 3, 7,
        // Top
        3, 2, 6,  3, 6, 7,
        // Bottom
        4, 5, 1,  4, 1, 0
    };

    // Create vertex buffer
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;

    device->CreateBuffer(&vbDesc, &vbData, &obj.vertexBuffer);

    // Create index buffer
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;

    device->CreateBuffer(&ibDesc, &ibData, &obj.indexBuffer);

    obj.indexCount = 36;
}

void Scene::createSphereGeometry(ID3D11Device* device, SceneObject& obj, int segments)
{
    const float radius = obj.size;
    std::vector<Vertex> vertices;
    std::vector<UINT> indices;

    // Generate sphere vertices (UV sphere)
    for (int lat = 0; lat <= segments; lat++)
    {
        float theta = (float)lat * M_PI / (float)segments;
        float sinTheta = sinf(theta);
        float cosTheta = cosf(theta);

        for (int lon = 0; lon <= segments; lon++)
        {
            float phi = (float)lon * 2.0f * M_PI / (float)segments;
            float sinPhi = sinf(phi);
            float cosPhi = cosf(phi);

            Vertex vertex;
            vertex.position[0] = radius * cosPhi * sinTheta;
            vertex.position[1] = radius * cosTheta;
            vertex.position[2] = radius * sinPhi * sinTheta;
            vertex.color[0] = obj.color.x;
            vertex.color[1] = obj.color.y;
            vertex.color[2] = obj.color.z;

            vertices.push_back(vertex);
        }
    }

    // Generate sphere indices
    for (int lat = 0; lat < segments; lat++)
    {
        for (int lon = 0; lon < segments; lon++)
        {
            UINT first = lat * (segments + 1) + lon;
            UINT second = first + segments + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    // Create vertex buffer
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = (UINT)(vertices.size() * sizeof(Vertex));
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices.data();

    device->CreateBuffer(&vbDesc, &vbData, &obj.vertexBuffer);

    // Create index buffer
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = (UINT)(indices.size() * sizeof(UINT));
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices.data();

    device->CreateBuffer(&ibDesc, &ibData, &obj.indexBuffer);

    obj.indexCount = (UINT)indices.size();
}

void Scene::compileShaders(ID3D11Device* device)
{
    // Compile vertex shader from file
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT hr = D3DCompileFromFile(
        L"shaders/VertexShader.hlsl",
        nullptr,
        nullptr,
        "main",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &vsBlob,
        &errorBlob
    );

    if (FAILED(hr) && errorBlob)
    {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        errorBlob->Release();
        return;
    }

    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);

    // Create input layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);
    vsBlob->Release();

    // Compile pixel shader
    ID3DBlob* psBlob = nullptr;
    hr = D3DCompileFromFile(
        L"shaders/PixelShader.hlsl",
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &psBlob,
        &errorBlob
    );

    if (FAILED(hr) && errorBlob)
    {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        errorBlob->Release();
        return;
    }

    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
    psBlob->Release();
}

void Scene::render(ID3D11DeviceContext* context, const vec3f& eyePosition, float elapsedTime)
{
    // Set shaders
    context->VSSetShader(m_vertexShader, nullptr, 0);
    context->PSSetShader(m_pixelShader, nullptr, 0);
    context->IASetInputLayout(m_inputLayout);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Calculate view-projection matrix with virtual screen at 400mm
    mat4f viewProjection = CalculateViewProjectionMatrix(
        eyePosition,
        m_screenWidthMM,
        m_screenHeightMM,
        m_virtualScreenDepthMM
    );

    // Render each object
    for (auto& obj : m_objects)
    {
        // Calculate model matrix (translation + rotation)
        float angle = elapsedTime * obj.rotationSpeed;
        mat4f rotation = mat4f::rotationY(angle);
        mat4f translation = mat4f::translation(obj.position);
        mat4f model = translation * rotation;

        // Update constant buffer
        D3D11_MAPPED_SUBRESOURCE mapped;
        context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        SceneConstants* constants = (SceneConstants*)mapped.pData;
        constants->viewProjection = viewProjection;
        constants->model = model;
        context->Unmap(m_constantBuffer, 0);

        context->VSSetConstantBuffers(0, 1, &m_constantBuffer);

        // Set vertex and index buffers
        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, &obj.vertexBuffer, &stride, &offset);
        context->IASetIndexBuffer(obj.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // Draw
        context->DrawIndexed(obj.indexCount, 0, 0);
    }
}

void Scene::shutdown()
{
    // Release object buffers
    for (auto& obj : m_objects)
    {
        if (obj.vertexBuffer) {
            obj.vertexBuffer->Release();
            obj.vertexBuffer = nullptr;
        }
        if (obj.indexBuffer) {
            obj.indexBuffer->Release();
            obj.indexBuffer = nullptr;
        }
    }
    m_objects.clear();

    // Release shader resources
    if (m_constantBuffer) {
        m_constantBuffer->Release();
        m_constantBuffer = nullptr;
    }
    if (m_vertexShader) {
        m_vertexShader->Release();
        m_vertexShader = nullptr;
    }
    if (m_pixelShader) {
        m_pixelShader->Release();
        m_pixelShader = nullptr;
    }
    if (m_inputLayout) {
        m_inputLayout->Release();
        m_inputLayout = nullptr;
    }
}
