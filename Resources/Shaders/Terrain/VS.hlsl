#include "Terrain.hlsl"

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID) {
	VertexOut vout = (VertexOut)0.0f;

    Instance instanceData = TerrainInstances[TerrainInstanceIdx];
    vin.Pos.y += TextureMaps[TerrainHeightMapIdx].SampleLevel(samAnisotropicWrap, vin.TexC, 0).r;
    vin.Pos.y *= (float)HeightFactor;

    float4 posW;

    posW = mul(float4(vin.Pos, 1.0f), instanceData.World);
    vout.Normal =  normalize(mul(vin.Normal, (float3x3)instanceData.World));
    vout.Tangent =  normalize(mul(vin.Tangent, (float3x3)instanceData.World));

    vout.PositionWorld = posW.xyz;
    vout.Position = mul(posW, ViewProj);
	vout.TexUV = mul(float4(vin.TexC, 0.0f, 1.0f), Materials[instanceData.MaterialIdx].MatTransform).xy;

    vout.ShadowPos = mul(posW, mul(Lights[0].View, Lights[0].Proj));
    vout.MaterialIdx = instanceData.MaterialIdx;
	
    return vout;
}