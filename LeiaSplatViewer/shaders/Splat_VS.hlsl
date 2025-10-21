/*!
 * Leia Splat Viewer
 * Gaussian Splat Vertex Shader
 */

cbuffer PerFrame : register(b0)
{
    matrix view;
    matrix projection;
    float3 eyePosition;
    float padding;
};

struct SplatVertex
{
    float3 position;
    float3 scale;
    float4 rotation;
    float opacity;
    float3 sh_dc;
    // Note: SH rest coefficients handled in constant buffer if needed
};

StructuredBuffer<SplatVertex> splats : register(t0);
StructuredBuffer<uint> sortedIndices : register(t1);

struct VS_OUTPUT
{
    float3 centerWorld : POSITION;
    float3 centerView : POSITION1;
    float3 scale : SCALE;
    float4 rotation : ROTATION;
    float opacity : OPACITY;
    float3 color : COLOR;
    uint vertexID : VERTEXID;
};

// Evaluate spherical harmonics (simplified - DC component only for now)
float3 evaluateSH(float3 sh_dc, float3 viewDir)
{
    // For now, just use DC component (constant color)
    // Full SH evaluation would be added here
    float3 color = sh_dc;

    // Clamp to positive values
    return max(color, 0.0);
}

VS_OUTPUT main(uint vertexID : SV_VertexID)
{
    VS_OUTPUT output;

    // Get splat index from sorted list
    uint splatIdx = sortedIndices[vertexID];
    SplatVertex splat = splats[splatIdx];

    // Store world position
    output.centerWorld = splat.position;

    // Transform to view space
    output.centerView = mul(view, float4(splat.position, 1.0)).xyz;

    // Convert scale from log space to linear
    output.scale = exp(splat.scale);

    // Pass through rotation (quaternion)
    output.rotation = splat.rotation;

    // Convert opacity from logit space using sigmoid
    output.opacity = 1.0 / (1.0 + exp(-splat.opacity));

    // Evaluate spherical harmonics for color
    float3 viewDir = normalize(eyePosition - splat.position);
    output.color = evaluateSH(splat.sh_dc, viewDir);

    // Pass vertex ID
    output.vertexID = vertexID;

    return output;
}
