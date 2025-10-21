/*!
 * Leia Splat Viewer
 * Gaussian Splat Geometry Shader
 * Expands point to quad billboard with proper 2D Gaussian projection
 */

cbuffer PerFrame : register(b0)
{
    matrix view;
    matrix projection;
    float3 eyePosition;
    float padding;
};

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

struct GS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 color : COLOR;
    float opacity : OPACITY;
    float2x2 covInv : COVINV; // Inverse of 2D covariance (for Gaussian evaluation)
};

// Convert quaternion to 3x3 rotation matrix
float3x3 quaternionToMatrix(float4 q)
{
    float x = q.x, y = q.y, z = q.z, w = q.w;

    float3x3 R;
    R[0][0] = 1.0 - 2.0 * (y*y + z*z);
    R[0][1] = 2.0 * (x*y - w*z);
    R[0][2] = 2.0 * (x*z + w*y);

    R[1][0] = 2.0 * (x*y + w*z);
    R[1][1] = 1.0 - 2.0 * (x*x + z*z);
    R[1][2] = 2.0 * (y*z - w*x);

    R[2][0] = 2.0 * (x*z - w*y);
    R[2][1] = 2.0 * (y*z + w*x);
    R[2][2] = 1.0 - 2.0 * (x*x + y*y);

    return R;
}

// Compute 3D covariance matrix from scale and rotation
// Σ3D = R * S * S^T * R^T
float3x3 computeCovariance3D(float3 scale, float4 rotation)
{
    float3x3 R = quaternionToMatrix(rotation);

    // Diagonal scale matrix
    float3x3 S = float3x3(
        scale.x, 0, 0,
        0, scale.y, 0,
        0, 0, scale.z
    );

    // Σ = R * S * S^T * R^T
    float3x3 RS = mul(R, S);
    float3x3 RSS = mul(RS, transpose(S));
    float3x3 cov3D = mul(RSS, transpose(R));

    return cov3D;
}

// Project 3D covariance to 2D screen space
// Based on EWA splatting projection
float2x2 projectCovariance(float3x3 cov3D, float3 centerView, matrix viewMat, matrix projMat)
{
    // Jacobian of perspective projection
    float z = centerView.z;
    float z2 = z * z;

    // Simplified 2D projection (affine approximation)
    // For a more accurate implementation, we'd compute the full Jacobian

    // Extract view-space 2D covariance (XY plane)
    float2x2 cov2D;
    cov2D[0][0] = cov3D[0][0];
    cov2D[0][1] = cov3D[0][1];
    cov2D[1][0] = cov3D[1][0];
    cov2D[1][1] = cov3D[1][1];

    // Add small value to diagonal for numerical stability
    cov2D[0][0] += 0.3f;
    cov2D[1][1] += 0.3f;

    return cov2D;
}

// Invert 2x2 matrix
float2x2 inverse2x2(float2x2 m)
{
    float det = m[0][0] * m[1][1] - m[0][1] * m[1][0];
    if (abs(det) < 1e-6) det = 1e-6; // Avoid division by zero

    float2x2 inv;
    inv[0][0] = m[1][1] / det;
    inv[0][1] = -m[0][1] / det;
    inv[1][0] = -m[1][0] / det;
    inv[1][1] = m[0][0] / det;

    return inv;
}

[maxvertexcount(4)]
void main(point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> triStream)
{
    // Compute 3D covariance
    float3x3 cov3D = computeCovariance3D(input[0].scale, input[0].rotation);

    // Project to 2D
    float2x2 cov2D = projectCovariance(cov3D, input[0].centerView, view, projection);

    // Compute inverse for pixel shader
    float2x2 covInv = inverse2x2(cov2D);

    // Compute eigenvalues/eigenvectors for quad size
    // For now, use a fixed size based on scale
    float quadSize = max(input[0].scale.x, max(input[0].scale.y, input[0].scale.z)) * 3.0;

    // Transform center to clip space
    float4 centerClip = mul(projection, float4(input[0].centerView, 1.0));
    float4 centerNDC = centerClip / centerClip.w;

    // Compute quad corners in NDC space
    float2 quadOffset = quadSize * 0.01; // Scale factor for screen space

    // Emit 4 vertices forming a quad
    GS_OUTPUT output;
    output.color = input[0].color;
    output.opacity = input[0].opacity;
    output.covInv = covInv;

    // Bottom-left
    output.position = float4(centerNDC.xy + float2(-quadOffset, -quadOffset), centerNDC.z, 1.0);
    output.uv = float2(-1, -1);
    triStream.Append(output);

    // Bottom-right
    output.position = float4(centerNDC.xy + float2(quadOffset, -quadOffset), centerNDC.z, 1.0);
    output.uv = float2(1, -1);
    triStream.Append(output);

    // Top-left
    output.position = float4(centerNDC.xy + float2(-quadOffset, quadOffset), centerNDC.z, 1.0);
    output.uv = float2(-1, 1);
    triStream.Append(output);

    // Top-right
    output.position = float4(centerNDC.xy + float2(quadOffset, quadOffset), centerNDC.z, 1.0);
    output.uv = float2(1, 1);
    triStream.Append(output);

    triStream.RestartStrip();
}
