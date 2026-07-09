#include "../Common/Common.hlsl"

struct UIVertexOut {
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
    uint TextureIdx : TEXINDEX;
};

StructuredBuffer<UIInstance> UIInstances     : register(t0, space1);