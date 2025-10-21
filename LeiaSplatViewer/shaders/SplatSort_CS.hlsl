/*!
 * Leia Splat Viewer
 * Depth Sorting Compute Shader
 * Sorts splats by depth for back-to-front rendering
 */

cbuffer SortParams : register(b0)
{
    float3 eyePosition;
    uint splatCount;
};

struct SplatVertex
{
    float3 position;
    float3 scale;
    float4 rotation;
    float opacity;
    float3 sh_dc;
    // Padding to match CPU layout
};

struct SplatDepth
{
    uint index;
    float depth;
};

StructuredBuffer<SplatVertex> splats : register(t0);
RWStructuredBuffer<SplatDepth> splatDepths : register(u0);

[numthreads(256, 1, 1)]
void ComputeDepths(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;

    if (idx >= splatCount)
        return;

    // Compute depth from eye position
    float3 toSplat = splats[idx].position - eyePosition;
    float depth = length(toSplat);

    // Store index and depth
    splatDepths[idx].index = idx;
    splatDepths[idx].depth = depth;
}

// Bitonic sort implementation (multiple passes)
cbuffer BitonicParams : register(b1)
{
    uint level;
    uint levelMask;
    uint width;
    uint height;
};

groupshared SplatDepth sharedData[512];

[numthreads(256, 1, 1)]
void BitonicSort(uint3 groupID : SV_GroupID,
                 uint3 groupThreadID : SV_GroupThreadID,
                 uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;

    // Load data into shared memory
    if (idx < splatCount)
    {
        sharedData[groupThreadID.x] = splatDepths[idx];
    }

    GroupMemoryBarrierWithGroupSync();

    // Bitonic sort step
    for (uint k = 2; k <= width; k <<= 1)
    {
        for (uint j = k >> 1; j > 0; j >>= 1)
        {
            uint ixj = idx ^ j;

            if (ixj > idx)
            {
                bool ascending = ((idx & k) == 0);

                SplatDepth a = sharedData[groupThreadID.x];
                SplatDepth b = sharedData[groupThreadID.x ^ j];

                // Sort by depth (back-to-front = descending)
                bool swap = ascending ? (a.depth < b.depth) : (a.depth > b.depth);

                if (swap)
                {
                    sharedData[groupThreadID.x] = b;
                    sharedData[groupThreadID.x ^ j] = a;
                }
            }

            GroupMemoryBarrierWithGroupSync();
        }
    }

    // Write back to global memory
    if (idx < splatCount)
    {
        splatDepths[idx] = sharedData[groupThreadID.x];
    }
}
