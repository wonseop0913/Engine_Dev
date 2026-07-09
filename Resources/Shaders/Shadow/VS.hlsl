#include "../Default/Default.hlsl"

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID) {
    VertexOut vout;

#ifdef SKINNED
    float3 posL = float3(0.0f, 0.0f, 0.0f);
    
    float boneWeights[4];
    boneWeights[0] = vin.BoneIndices[0] == -1 ? 0 : vin.BoneWeights[0];
    boneWeights[1] = vin.BoneIndices[1] == -1 ? 0 : vin.BoneWeights[1];
    boneWeights[2] = vin.BoneIndices[2] == -1 ? 0 : vin.BoneWeights[2];
    boneWeights[3] = vin.BoneIndices[3] == -1 ? 0 : vin.BoneWeights[3];

    float weightNormalizeValue = 1.0f / (boneWeights[0] + boneWeights[1] + boneWeights[2] + boneWeights[3]);

    for (int i = 0; i < 4; i++)
    {
        posL += boneWeights[i] * weightNormalizeValue * mul(float4(vin.Pos, 1.0f), BoneTransforms[vin.BoneIndices[i]]).xyz;
    }

    vin.Pos = posL;
#endif

    Instance instanceData = Instances[instanceID + InstanceStartIndex];

#ifdef SKINNED
    float4 posW = float4(vin.Pos, 1.0f);
#else
    float4 posW = mul(float4(vin.Pos, 1.0f), instanceData.World);
#endif
    vout.Position = mul(posW, mul(Lights[0].View, Lights[0].Proj));

    return vout;
}