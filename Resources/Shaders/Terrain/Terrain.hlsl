#include "../Common/Lighting.hlsl"

cbuffer TerrainInfo : register(b0, space1) {
    uint TerrainInstanceIdx;
    uint TerrainTextureIdx;
    uint TerrainHeightMapIdx;
    float HeightFactor;
};

StructuredBuffer<Instance> TerrainInstances     : register(t0, space1);