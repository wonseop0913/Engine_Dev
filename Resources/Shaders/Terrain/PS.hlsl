#include "Terrain.hlsl"

float4 PS(VertexOut pin) : SV_TARGET {
    Material mat = Materials[pin.MaterialIdx];

    float3 eyePos = GetCameraPosition();
    float4 albedo = TextureMaps[TerrainTextureIdx].Sample(samLinearWrap, pin.TexUV * mat.Tilling);
    float3 eyeDir = normalize(eyePos - pin.PositionWorld);
    pin.Normal = normalize(pin.Normal);

    if (mat.NormalMapIndex != 0) {
        float3 normalMapSample = TextureMaps[mat.NormalMapIndex].Sample(samAnisotropicWrap, pin.TexUV * mat.Tilling).rgb;
        float3 normalT = 2.0 * normalMapSample - 1.0;

        float3 N = pin.Normal;
        float3 T = normalize(pin.Tangent - dot(pin.Tangent, N) * N);
        float3 B = cross(N, T);

        float3x3 TBN = float3x3(T, B, N);
        pin.Normal = mul(normalT, TBN);
    }

    float4 result = BRDFLighting(mat, albedo, pin, eyeDir);
    result.rgb = result.rgb * (1.0 - CameraColorBlend.a) + CameraColorBlend.rgb * CameraColorBlend.a;

    return result;
}