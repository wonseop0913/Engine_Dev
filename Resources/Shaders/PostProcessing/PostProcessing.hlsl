#include "../Common/Common.hlsl"

struct PostProcessingVertexOut {
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD;
};

Texture2D   MainPassRenderResult        : register(t0, space1);

// 나중에 Editor 매크로 작성으로 용도 분류 하는 것 고려
Texture2D   OutlinePassRenderResult     : register(t0, space2);