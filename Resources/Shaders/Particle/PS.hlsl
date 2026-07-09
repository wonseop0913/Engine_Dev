#include "Particle.hlsl"

float4 PS(GS_OUT input) : SV_Target {
    Texture2D tex = TextureMaps[input.texIndex];
    float4 texColor = tex.Sample(samAnisotropicWrap, input.uv);

    float4 result = texColor * input.color;
    result.rgb = result.rgb * (1.0 - CameraColorBlend.a) + CameraColorBlend.rgb * CameraColorBlend.a;

    return result;
}