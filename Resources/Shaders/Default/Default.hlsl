#include "../Common/Common.hlsl"

StructuredBuffer<Instance> Instances            : register(t0, space1);
StructuredBuffer<float4x4> BoneTransforms       : register(t1, space1);
