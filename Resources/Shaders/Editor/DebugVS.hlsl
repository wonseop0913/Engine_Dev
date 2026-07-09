#include "../Default/Default.hlsl"

VertexPosColorOut VS(VertexPosColorIn vin) {
    VertexPosColorOut vout;

    // float dummy = (float)InstanceStartIndex * 0.000001f;

    vout.Position = mul(float4(vin.Pos, 1.0f), ViewProj);
    vout.Color = vin.Color;

    return vout;
}