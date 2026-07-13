#include "../Common/Common.hlsl"

struct PostProcessingVertexOut {
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD;
};

// Texture2D   MainPassRenderResult        : register(t0, space1);