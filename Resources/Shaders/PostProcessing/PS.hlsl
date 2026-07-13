#include "../PostProcessing/PostProcessing.hlsl"

float4 PS(PostProcessingVertexOut input) : SV_Target {
    float4 result = MainPassRenderResult.Sample(samLinearClamp, input.UV);

    float2 texelSize = InvRenderTargetSize * 3;

    if (OutlinePassRenderResult.Sample(samLinearClamp, input.UV).r < 1) {
        float val = 0;
        [unroll]
        for (int y = -1; y <= 1; ++y) {
            [unroll]
            for (int x = -1; x <= 1; ++x) {
                float2 offset = float2(x, y) * texelSize;
                val += OutlinePassRenderResult.Sample(samLinearClamp, input.UV + offset).r;
            }
        }

        if (val > 0) {
            result = float4(1.0, 0.6, 0.0, 1.0);
        }
    }

    return result;
}