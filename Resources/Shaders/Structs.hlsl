#define MAX_KEYFRAME_COUNT 300
#define MAX_BONE_COUNT 250

/****************/
/* Skinned Mesh */
/****************/

struct BoneTransform {
    float4x4 transform;
};

struct KeyFrame {
    float4x4 transform;
};

struct AnimationData {
    KeyFrame keyFrames[MAX_KEYFRAME_COUNT];
    uint keyFrameCount;
};

struct Animation {
    AnimationData animationDatas[MAX_BONE_COUNT];
};

/****************/
/* Buffer Stuff */
/****************/

struct Light {
    float4x4 View;
    float4x4 Proj;
    float4   Diffuse;
    float3   Position;
    int      LightType;
    float3   Direction;
    float    SpotPower;
    float    FallOffStart;
    float    FallOffEnd;
    float    Intensity;
    float    padding1;
};

struct Instance {
    float4x4 World;
    float4x4 WorldInv;
    uint MaterialIdx;
    float3 padding;
};

struct Material {
    float4x4 MatTransform;
    float4   Ambient;
    float4   Diffuse;
    float4   Specular;
    float4   Emissive;
    float2   Tilling;
    float    Metallic;
    float    Roughness;
    uint     DiffuseMapIndex;
    uint     NormalMapIndex;
    uint     MetallicMapIndex;
    uint     RoughnessMapIndex;
    uint     SpecularMapIndex;
    float3   padding1;
};

struct Particle {
    float3  Position;
    float3  Velocity;
    float   Age;
    float   LifeTime;
    uint    TextureIndex;
    float2  Size;
    float4  Color;
};

struct UIInstance {
    float2  CenterPos;
    float2  Size;
    float   Depth;
    uint    TextureIndex;
    float4  Color;
};

/**********/
/* Vertex */
/**********/

struct VertexIn {
	float3 Pos          : POSITION;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
	float2 TexC         : TEXCOORD;
#ifdef SKINNED
    float4 BoneWeights  : WEIGHTS;
    int4 BoneIndices    : BONEINDICES;
#endif
};

struct VertexOut {
	float4 Position         : SV_POSITION;
    float3 PositionWorld    : POSITION;
    float3 Normal           : NORMAL;
    float3 Tangent          : TANGENT;
	float2 TexUV            : TEXCOORD2;
    float4 ShadowPos        : TEXCOORD1;
    uint MaterialIdx        : MATINDEX;
};

struct VertexPosColorIn {
    float3 Pos          : POSITION;
    float4 Color        : COLOR;
};

struct VertexPosColorOut {
    float4 Position     : SV_POSITION;
    float4 Color        : COLOR;
};

struct VertexPosTexIn {
    float3 PosL : POSITION;
    float2 TexC : TEXCOORD;
};

struct VertexPosTexOut {
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD;
};

struct VertexPosIn {
    float3 Pos : POSITION;
};

struct VertexPosOut {
    float4 Pos : SV_POSITION;
};