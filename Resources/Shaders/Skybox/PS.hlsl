#include "Skybox.hlsl"

float4 PS(VertexSkyboxOut pin) : SV_Target {

    float4 result = CubeMap.Sample(samLinearWrap, pin.LocalPosition);
    result.rgb = result.rgb * (1.0 - CameraColorBlend.a) + CameraColorBlend.rgb * CameraColorBlend.a;

    return result;
}