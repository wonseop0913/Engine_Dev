#include "../Common/Common.hlsl"

struct VS_OUT {
    float3  Position : POSITION;
    float4  Color : COLOR;
    uint    TexIndex : TEXINDEX;
    float2  Size : SIZE;
};

struct GS_OUT {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR;
    uint texIndex : TEXINDEX;
};

// Particle
RWStructuredBuffer<Particle> particles : register(u0, space1);

cbuffer cbParticleEmitterInfo : register(b0, space1) {
    float3  EmitterPos;
    float   ParticleInitVelocity;
    float2  ParticleSize;
    float   GravityFactor;
    float   ParticleLifeTime;
    float4  InitialColor;
    float   SpawnRate;
    uint    SpawnMount;
    uint    StartIdx;
    uint    TextureIdx;
    float3  EmitDirection;
    float   ConeAngle;
    uint    EmitterShape;
};