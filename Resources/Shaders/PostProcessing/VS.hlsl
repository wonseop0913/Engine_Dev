#include "../PostProcessing/PostProcessing.hlsl"

PostProcessingVertexOut VS(VertexIn input, uint instanceID : SV_InstanceID) {
    PostProcessingVertexOut result;

    // Default quad position range is between -0.5f ~ 0.5f
    result.Pos = float4(input.Pos * 2.0f, 1.0f);
    result.UV = input.TexC;

    return result;
}