/*!
 * LookAround Eyes Comparison App
 * Scene management - creates and renders 3D objects at different depths
 */

#pragma once

#include "Math.h"
#include <d3d11.h>
#include <vector>

// Vertex format for our geometry
struct Vertex
{
    float position[3];  // XYZ
    float color[3];     // RGB
};

// Constant buffer for transforms
struct SceneConstants
{
    mat4f viewProjection;
    mat4f model;
};

// Types of geometry
enum class ObjectType
{
    Cube,
    Sphere
};

// Scene object descriptor
struct SceneObject
{
    vec3f position;           // World position (mm)
    vec3f color;              // RGB color
    float size;               // Size in mm
    float rotationSpeed;      // Radians per second
    ObjectType type;

    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    UINT indexCount;

    SceneObject()
        : position(0.0f, 0.0f, 0.0f)
        , color(1.0f, 1.0f, 1.0f)
        , size(1.0f)
        , rotationSpeed(0.0f)
        , type(ObjectType::Cube)
        , vertexBuffer(nullptr)
        , indexBuffer(nullptr)
        , indexCount(0)
    {}
};

// Scene manager
class Scene
{
public:
    Scene();
    ~Scene();

    // Initialize scene with display parameters
    void initialize(ID3D11Device* device,
                    float screenWidthMM,
                    float screenHeightMM,
                    float virtualScreenDepthMM);

    // Render all objects
    void render(ID3D11DeviceContext* context,
                const vec3f& eyePosition,
                float elapsedTime);

    // Cleanup
    void shutdown();

private:
    // Helper functions
    void createNearPlaneObjects(ID3D11Device* device);
    void createMidPlaneObjects(ID3D11Device* device);
    void createFarPlaneObjects(ID3D11Device* device);

    void createCubeGeometry(ID3D11Device* device, SceneObject& obj);
    void createSphereGeometry(ID3D11Device* device, SceneObject& obj, int segments = 20);

    void compileShaders(ID3D11Device* device);

    // Scene data
    std::vector<SceneObject> m_objects;

    // Display parameters
    float m_screenWidthMM;
    float m_screenHeightMM;
    float m_virtualScreenDepthMM;

    // DirectX resources
    ID3D11Buffer* m_constantBuffer;
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_inputLayout;
};
