#include "../Common/Structs.hlsl"

#pragma once

/************/
/* Samplers */
/************/

SamplerState samPointWrap            : register(s0);
SamplerState samPointClamp           : register(s1);
SamplerState samLinearWrap           : register(s2);
SamplerState samLinearClamp          : register(s3);
SamplerState samAnisotropicWrap      : register(s4);
SamplerState samAnisotropicClamp     : register(s5);
SamplerComparisonState samShadow     : register(s6);


/***********/
/* Buffers */
/***********/

StructuredBuffer<Material> Materials    : register(t0);
StructuredBuffer<Light> Lights          : register(t1);
TextureCube CubeMap                     : register(t2);
Texture2D   ShadowMap                   : register(t3);
Texture2D   TextureMaps[100]            : register(t4);

cbuffer ClientInfo : register(b0) {
    float   DeltaTime;
    float   Time;
}

cbuffer LightInfo : register(b1) {
    uint gNumLights;
    uint BRDFLutTextureIdx;
}

cbuffer cbCamera : register(b2) {
    float4x4 View;
    float4x4 ViewInv;
    float4x4 Proj;
    float4x4 ProjInv;
    float4x4 ViewProj;
    float4x4 ViewProjInv;
    float4x4 Ortho;
    float2 RenderTargetSize;
    float2 InvRenderTargetSize;
    float4 CameraColorBlend;
};

cbuffer MeshInfo : register(b3) {
    uint InstanceStartIndex;
    uint InstanceOffset;    // Only using for editor outline render now
};


/*************/
/* Functions */
/*************/

float3 GetCameraPosition() {
    return ViewInv._41_42_43;
}

float3 GetCameraRight() {
    return float3(ViewInv._11, ViewInv._12, ViewInv._13);
}

float3 GetCameraUp() {
    return float3(ViewInv._21, ViewInv._22, ViewInv._23);
}

float3 GetCameraForward() {
    return float3(ViewInv._31, ViewInv._32, ViewInv._33);
}