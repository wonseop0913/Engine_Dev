#include "../Common/Structs.hlsl"

float4 PS(VertexPosColorOut pin) : SV_Target {
    return pin.Color;
}