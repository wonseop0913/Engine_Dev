#include "../PostProcessing/PostProcessing.hlsl"

float4 PS(PostProcessingVertexOut input) : SV_Target {

    // float4 result = TextureMaps[MainPassRenderResultIdx].Sample(samLinearClamp, input.UV);
    float4 result = MainPassRenderResult.Sample(samLinearClamp, input.UV);

    return result;
}