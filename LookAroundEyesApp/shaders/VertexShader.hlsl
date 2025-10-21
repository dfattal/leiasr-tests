// Vertex Shader for LookAround Eyes App
// Transforms vertices using view-projection and model matrices

cbuffer Constants : register(b0)
{
    matrix viewProjection;  // Combined view-projection matrix from eye
    matrix model;           // Model transformation (rotation, scale, translation)
};

struct VS_INPUT
{
    float3 position : POSITION;
    float3 color    : COLOR;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 color    : COLOR;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // Transform to world space
    float4 worldPos = mul(model, float4(input.position, 1.0));

    // Transform to clip space using view-projection
    output.position = mul(viewProjection, worldPos);

    // Pass through color
    output.color = input.color;

    return output;
}
