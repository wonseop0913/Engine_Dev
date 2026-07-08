#include "Default.hlsl"

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID) {
	VertexOut vout = (VertexOut)0.0f;

#ifdef SKINNED
    float3 posL = float3(0.0f, 0.0f, 0.0f);
    float3 normalL = float3(0.0f, 0.0f, 0.0f);
    float3 tangentL = float3(0.0f, 0.0f, 0.0f);
    
    float boneWeights[4];
    boneWeights[0] = vin.BoneIndices[0] == -1 ? 0 : vin.BoneWeights[0];
    boneWeights[1] = vin.BoneIndices[1] == -1 ? 0 : vin.BoneWeights[1];
    boneWeights[2] = vin.BoneIndices[2] == -1 ? 0 : vin.BoneWeights[2];
    boneWeights[3] = vin.BoneIndices[3] == -1 ? 0 : vin.BoneWeights[3];

    float weightNormalizeValue = 1.0f / (boneWeights[0] + boneWeights[1] + boneWeights[2] + boneWeights[3]);

    for (int i = 0; i < 4; i++)
    {
        posL +=     boneWeights[i] * weightNormalizeValue * mul(float4(vin.Pos, 1.0f), BoneTransforms[vin.BoneIndices[i]]).xyz;
        normalL +=  boneWeights[i] * weightNormalizeValue * mul(vin.Normal, (float3x3)BoneTransforms[vin.BoneIndices[i]]);
        tangentL += boneWeights[i] * weightNormalizeValue * mul(vin.Tangent.xyz, (float3x3)BoneTransforms[vin.BoneIndices[i]]);
    }

    vin.Pos = posL;
    vin.Normal = normalL;
    vin.Tangent = tangentL;
#endif

    Instance instanceData = Instances[instanceID + InstanceStartIndex];
    if (InstanceOffset != -1)
        instanceData = Instances[InstanceStartIndex + InstanceOffset];

    float4 posW;

#ifdef SKINNED
    posW = float4(vin.Pos, 1.0f);
    vout.Normal =  vin.Normal;
    vout.Tangent = vin.Tangent;
#else
    posW = mul(float4(vin.Pos, 1.0f), instanceData.World);
    vout.Normal =  normalize(mul(vin.Normal, (float3x3)instanceData.World));
    vout.Tangent =  normalize(mul(vin.Tangent, (float3x3)instanceData.World));
#endif
    vout.PositionWorld = posW.xyz;
    vout.Position = mul(posW, ViewProj);
	vout.TexUV = mul(float4(vin.TexC, 0.0f, 1.0f), Materials[instanceData.MaterialIdx].MatTransform).xy;

    vout.ShadowPos = mul(posW, mul(Lights[0].View, Lights[0].Proj));
    vout.MaterialIdx = instanceData.MaterialIdx;
	
    return vout;
}