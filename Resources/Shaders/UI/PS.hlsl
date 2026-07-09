#include "UI.hlsl"

float4 PS(UIVertexOut input) : SV_Target {

    float4 result = TextureMaps[input.TextureIdx].Sample(samLinearClamp, input.UV) * input.Color;
    result.rgb = result.rgb * (1.0 - CameraColorBlend.a) + CameraColorBlend.rgb * CameraColorBlend.a;

    return result;
}