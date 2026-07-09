#include "UI.hlsl"

UIVertexOut VS(VertexIn input, uint instanceID : SV_InstanceID) {
    UIVertexOut result;
    
    UIInstance data = UIInstances[instanceID];
    
    float3 worldPos = input.Pos * float3(data.Size, 1.0f) + float3(data.CenterPos, data.Depth);
    
    result.Pos = mul(float4(worldPos, 1.0f), Ortho);
    result.UV = input.TexC;
    result.Color = data.Color;
    result.TextureIdx = data.TextureIndex;
    return result;
}