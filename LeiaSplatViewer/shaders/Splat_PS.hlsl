/*!
 * Leia Splat Viewer
 * Gaussian Splat Pixel Shader
 * Evaluates 2D Gaussian and outputs color with alpha
 */

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 color : COLOR;
    float opacity : OPACITY;
    float2x2 covInv : COVINV;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // Evaluate 2D Gaussian
    // G(x) = exp(-0.5 * x^T * Σ^(-1) * x)

    float2 d = input.uv;

    // Compute d^T * Σ^(-1) * d
    float2 temp = mul(input.covInv, d);
    float power = -0.5 * dot(d, temp);

    // Gaussian value
    float gaussian = exp(power);

    // Final alpha
    float alpha = input.opacity * gaussian;

    // Early discard for very low alpha (optimization)
    if (alpha < 0.01)
        discard;

    // Pre-multiplied alpha for correct blending
    float3 colorPremult = input.color * alpha;

    return float4(colorPremult, alpha);
}
