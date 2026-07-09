#include "Particle.hlsl"

[maxvertexcount(4)]
void GS(point VS_OUT input[1], inout TriangleStream<GS_OUT> triStream) {
    if (input[0].Size.x == 0 && input[0].Size.y == 0) return;
    
    float3 center = input[0].Position;
    float2 size = input[0].Size;
    float3 right = normalize(GetCameraRight()) * size.x;
    float3 up = normalize(GetCameraUp()) * size.y;

    // 4개의 billboard vertex 생성
    float3 offsets[4] = {
        -right - up, // bottom-left
        -right + up, // top-left
         right - up, // bottom-right
         right + up  // top-right
    };

    float2 uvs[4] = {
        float2(0, 1),
        float2(0, 0),
        float2(1, 1),
        float2(1, 0)
    };

    for (int i = 0; i < 4; i++)
    {
        GS_OUT v;
        v.pos = mul(float4(center + offsets[i], 1.0f), mul(View, Proj));
        v.uv = uvs[i];
        v.color = input[0].Color;
        v.texIndex = input[0].TexIndex;
        triStream.Append(v);
    }

    triStream.RestartStrip();
}