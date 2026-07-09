#include "../Default/Default.hlsl"

struct VertexSkyboxIn {
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float3 Tangent  : TANGENT;
    float2 TexC     : TEXCOORD;
};

struct VertexSkyboxOut {
    float4 Position         : SV_POSITION;
    float3 LocalPosition    : POSITION;
};