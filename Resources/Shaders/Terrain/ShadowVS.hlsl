#include "Terrain.hlsl"

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID) {
    VertexOut vout;

    Instance instanceData = TerrainInstances[TerrainInstanceIdx];
    vin.Pos.y += TextureMaps[TerrainHeightMapIdx].SampleLevel(samAnisotropicWrap, vin.TexC, 0).r;
    vin.Pos.y *= (float)HeightFactor;

    float4 posW = mul(float4(vin.Pos, 1.0f), instanceData.World);
    vout.Position = mul(posW, mul(Lights[0].View, Lights[0].Proj));

    return vout;
}