#include "Particle.hlsl"

uint Hash(uint n)
{
    // n = (n << 13U) ^ n;
    // return (1.0f - ((n * (n * n * 15731U + 789221U) + 1376312589U) & 0x7fffffffU) / 1073741824.0f);

    n = ((n >> 16) ^ n) * 0x45d9f3b;
    n = ((n >> 16) ^ n) * 0x45d9f3b;
    n = (n >> 16) ^ n;
    return n;
}

// 해시를 이용해 2D 또는 3D 난수 벡터를 만드는 함수
float3 RandomDir(uint instanceID, float time)
{
    uint seed = instanceID ^ Hash((uint)(time * 1000.0f));
    
    uint hX = Hash(seed);
    uint hY = Hash(seed + 192837111U); // 인접하지 않은 큰 소수를 더함
    uint hZ = Hash(seed + 374761393U);

    float3 v = float3(float(hX) / 4294967295.0f, float(hY) / 4294967295.0f, float(hZ) / 4294967295.0f) * 2.0f - 1.0f;

    if (length(v) < 0.000001f) return float3(0, 1, 0);

    // 단위 벡터로 정규화
    return normalize(v);
}

float3 RandomDir(uint instanceID, float time, float maxAngle, float3 direction = {0, 1, 0})
{
    uint seed = instanceID ^ Hash((uint)(time * 1000.0f));
    
    // 1. 구면 좌표계(Spherical Coordinates)를 이용한 방식
    float u = float(Hash(seed)) / 4294967295.0f;      // 0 ~ 1
    float v = float(Hash(seed + 12345U)) / 4294967295.0f; // 0 ~ 1

    // maxAngle을 라디안으로 변환 (예: 20도 -> 0.34rad)
    float phi = 2.0f * 3.141592f * u; // 한 바퀴 회전
    float cosTheta = 1.0f - v * (1.0f - cos(maxAngle)); // 각도 제한
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

    // 원뿔 내부의 로컬 방향
    float3 localDir = float3(cos(phi) * sinTheta, cosTheta, sin(phi) * sinTheta);
    if (direction.y > 0.999f) return normalize(localDir);
    
    float3 up = abs(direction.z) < 0.999f ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangent = normalize(cross(up, direction));
    float3 binormal = cross(direction, tangent);
    return normalize(tangent * localDir.x + direction * localDir.y + binormal * localDir.z);
}

[numthreads(256, 1, 1)]
void CS(int3 threadId : SV_DispatchThreadID) {
    uint id = threadId.x;
    if (id >= 1024) return;     // TEMP - 버퍼 최대 크기 따로 받아야함

    Particle p = particles[id];

    bool isNew = false;
    uint endIdx = StartIdx + SpawnMount;

    if (id >= StartIdx && id < endIdx) isNew = true;        // 최대 크기 넘어가는건 위에서 걸러서 신경 안써도 됨
    if (endIdx >= 1024 && id < (endIdx % 1024)) isNew = true;

    // 새로 생성할 파티클인 경우
    if (isNew) {
        float3 dir;
        if (EmitterShape == 0)
            dir = RandomDir(id, Time);
        else if (EmitterShape == 1)
            dir = RandomDir(id, Time, ConeAngle * 3.141592 / 180.0, EmitDirection);
        p.Position = EmitterPos;
        p.Velocity = ParticleInitVelocity * dir;
        p.Age = 0.0f;
        p.LifeTime = ParticleLifeTime;
        p.TextureIndex = TextureIdx;
        p.Size = ParticleSize;
        p.Color = InitialColor;
    }
    // 원래 있었고 살아있는 애
    else if (p.Age < p.LifeTime) {
        p.Velocity.y += GravityFactor * DeltaTime;
        p.Position += p.Velocity * DeltaTime;
        p.Age += DeltaTime;

        p.Color = lerp(InitialColor, float4(0.0f, 0.0f, 0.0f, 0.0f), pow(p.Age / p.LifeTime, 10));
    }
    // 원래 있었고 죽은 애
    else {
        p.Size = float2(0.0f, 0.0f);
    }

    particles[id] = p;
}