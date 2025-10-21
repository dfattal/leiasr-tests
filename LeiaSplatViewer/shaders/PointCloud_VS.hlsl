/*!
 * Leia Splat Viewer
 * Point Cloud Vertex Shader
 */

cbuffer PerFrame : register(b0)
{
    matrix viewProjection;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float3 color : COLOR;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // Transform to clip space
    output.position = mul(viewProjection, float4(input.position, 1.0));

    // Pass through color
    output.color = input.color;

    return output;
}
