#include "../Common/Common.hlsl"

#define MIPMAP_LEVEL_MAX    10

// Legacy Blinh Phong
// float4 ProcessAmbient(float4 ambient, float4 albedo) {
//     return ambient * albedo;
// }

// float4 ProcessDiffuse(float4 diffuse, float4 albedo, float3 lightDir, float3 normal) {
//     float4 totalDiffuse;

//     float diffuseValue = dot(lightDir, normal);

//     totalDiffuse = diffuse * albedo * diffuseValue * diffuse.a;

//     return totalDiffuse;
// }

// float4 ProcessSpecular(float4 specular, float shiness, float3 lightDir, float3 normal, float3 eyeDir) {
//     float4 totalSpecular;

//     float3 reflection = reflect(lightDir, normal);
//     float specularValue = saturate(dot(reflection, eyeDir));
//     specularValue = pow(specularValue, 10);

//     totalSpecular = specular * specularValue * shiness;

//     return totalSpecular;
// }

// float4 ProcessEmissive(float4 emissive, float4 albedo) {
//     float4 totalEmissive;

//     totalEmissive = emissive * albedo;
//     totalEmissive.a = 0.0f;

//     return totalEmissive;
// }

// float4 ComputeLight(Material mat, float4 albedo, float3 normal, float3 eyeDir) {
//     float4 totalColor = { 0.0, 0.0, 0.0, 0.0 };

//     // Global Light Process
//     {
//         float3 lightDir = normalize(-Lights[0].Direction);

//         float4 ambient = ProcessAmbient(Lights[0].Ambient * mat.Ambient, albedo);
//         float4 diffuse = ProcessDiffuse(Lights[0].Diffuse * mat.Diffuse, albedo, lightDir, normal);
//         float4 specular = ProcessSpecular(Lights[0].Specular * mat.Specular, mat.Metallic, lightDir, normal, eyeDir);
//         float4 emissive = ProcessEmissive(mat.Emissive, albedo);

//         totalColor = ambient + diffuse + specular;
//     }

//     // // General Light Process
//     // {
//     //     for (int i = 1; i < gNumLights; i++)
//     //     {
//     //         float4 diffuse = ProcessDiffuse(Lights[i].Diffuse * mat.Diffuse, albedo, Lights[i].Direction, normal);
//     //         float4 specular = ProcessSpecular(Lights[i].Specular * mat.Specular, mat.Shiness, Lights[i].Direction, normal, eyeDir);
        
//     //         totalColor += diffuse;
//     //     }
//     // }

//     float4 emissive = ProcessEmissive(mat.Emissive, albedo);
//     totalColor += emissive;

//     return totalColor;
// }

float DistributionGGX(float NdotH, float roughness) {
    float a2 = pow(roughness, 4.0);
    float NdotH2 = pow(max(NdotH, 0.0), 2.0);
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = acos(-1) * denom * denom;
    return a2 / denom;
}

float GeometrySmith(float NdotV, float NdotL, float roughness) {
    //float k = roughness * roughness / 2.0f;
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;

    float GV = max(NdotV, 0.0) / (NdotV * (1.0 - k) + k);
    float GL = max(NdotL, 0.0) / (NdotL * (1.0 - k) + k);
    
    return GV * GL;
}

float3 FresnelSchlick(float3 F0, float NdotV) {
    return F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);
}

float3 FresnelSchlick(float3 F0, float NdotV, float roughness) {
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(1.0 - NdotV, 5.0);
}

float3 ACESToneMap(float3 color) {
    const float A = 2.51, B = 0.03, C = 2.43, D = 0.59, E = 0.14;
    return saturate((color * (A * color + B)) / (color * (C * color + D) + E));
}

// PCF 3x3
float CalcDirectionalLightShadow(Light light, VertexOut pixel) {
    float shadowFactor;

    float4 worldPos = float4(pixel.PositionWorld, 1.0f);
    float3 lightDir = light.Direction;

    float bias = 0.001;
    float3 shadowMapTex = pixel.ShadowPos.xyz / pixel.ShadowPos.w;
    shadowMapTex.x = shadowMapTex.x / 2.0 + 0.5;
    shadowMapTex.y = -shadowMapTex.y / 2.0 + 0.5;

    float depthValue = ShadowMap.Sample(samLinearClamp, shadowMapTex.xy).r;
    float lightDepthValue = shadowMapTex.z;
    lightDepthValue = lightDepthValue - bias;

    if (shadowMapTex.x < 0.0f || shadowMapTex.x > 1.0f || 
        shadowMapTex.y < 0.0f || shadowMapTex.y > 1.0f || 
        lightDepthValue > 1.0f) {
        shadowFactor = 1;
    }
    else {
        uint width, height, numMips;
        ShadowMap.GetDimensions(0, width, height, numMips);

        float dx = 1.0f / (float)width;
        const float2 offsets[9] =
        {
            float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
            float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
            float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
        };

        float percentLit = 0.0f;
        [unroll]
        for (int i = 0; i < 9; ++i) {
            percentLit += ShadowMap.SampleCmpLevelZero(samShadow,
                shadowMapTex.xy + offsets[i], lightDepthValue).r;
        }

        shadowFactor = percentLit / 9;
    }

    return shadowFactor;
}

float3 ComputeDirectionalLight(Light light, Material mat, float4 albedo, VertexOut pixel, float3 V, float NdotV, float3 F0, float3 metallicValue, float roughnessValue, float3 specularValue) {
    float3 L = normalize(-light.Direction);
    float3 H = normalize(V + L);

    float NdotL = dot(pixel.Normal, L);
    float NdotH = dot(pixel.Normal, H);
    float HdotV = dot(H, V);

    float D = DistributionGGX(NdotH, roughnessValue);
    float G = GeometrySmith(NdotV, NdotL, roughnessValue);
    float3 F = FresnelSchlick(F0, HdotV);

    float3 nom = D * G * F;
    float denom = 4 * max(dot(L, pixel.Normal), 0.0) * max(dot(V, pixel.Normal), 0.0) + 0.0001;
    float3 specular = nom / max(denom, 0.0001);
    specular *= specularValue;

    float3 kS = F;
    float3 kD = (float3(1.0, 1.0, 1.0) - kS) * (float3(1.0, 1.0, 1.0) - metallicValue);

    float3 diffuse = kD * albedo.rgb * mat.Diffuse.rgb / acos(-1);
    return (diffuse + specular) * light.Diffuse.rgb * max(dot(pixel.Normal, L), 0.0);
}

float3 ComputePointLight(Light light, Material mat, float4 albedo, VertexOut pixel, float3 V, float NdotV, float3 F0, float3 metallicValue, float roughnessValue, float3 specularValue) {
    float3 L = light.Position - pixel.PositionWorld;
    float len = length(L);
    L = normalize(L);

    if (len > light.FallOffEnd) return float3(0, 0, 0);

    float att = saturate((light.FallOffEnd - len) / (light.FallOffEnd - light.FallOffStart));
    
    float3 H = normalize(V + L);

    float NdotL = dot(pixel.Normal, L);
    float NdotH = dot(pixel.Normal, H);
    float HdotV = dot(H, V);

    float D = DistributionGGX(NdotH, roughnessValue);
    float G = GeometrySmith(NdotV, NdotL, roughnessValue);
    float3 F = FresnelSchlick(F0, HdotV);

    float3 nom = D * G * F;
    float denom = 4 * max(dot(L, pixel.Normal), 0.0) * max(dot(V, pixel.Normal), 0.0) + 0.0001;
    float3 specular = nom / max(denom, 0.0001);
    specular *= specularValue;

    float3 kS = F;
    float3 kD = (float3(1.0, 1.0, 1.0) - kS) * (float3(1.0, 1.0, 1.0) - metallicValue);

    float3 diffuse = kD * albedo.rgb * mat.Diffuse.rgb / acos(-1);
    return (diffuse + specular) * light.Diffuse.rgb * max(dot(pixel.Normal, L), 0.0) * att;
}

float4 BRDFLighting(Material mat, float4 albedo, VertexOut pixelIn, float3 V) {
    float3 color = { 0.0, 0.0, 0.0 };

    float3 metallicValue = { mat.Metallic, mat.Metallic, mat.Metallic };
    if (mat.MetallicMapIndex != 0)
        metallicValue = TextureMaps[mat.MetallicMapIndex].Sample(samAnisotropicWrap, pixelIn.TexUV * mat.Tilling).rgb;

    float roughnessValue = mat.Roughness;
    if (mat.RoughnessMapIndex != 0)
        roughnessValue = TextureMaps[mat.RoughnessMapIndex].Sample(samAnisotropicWrap, pixelIn.TexUV * mat.Tilling).r;
    roughnessValue = max(roughnessValue, 0.045);

    float3 specularValue = { 1.0, 1.0, 1.0 };
    if (mat.SpecularMapIndex != 0)
        specularValue *= TextureMaps[mat.SpecularMapIndex].Sample(samAnisotropicWrap, pixelIn.TexUV * mat.Tilling).rgb;


    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo.rgb * mat.Diffuse.rgb, metallicValue);

    float NdotV = dot(pixelIn.Normal, V);

    Light currentLight;
    for (int i = 0; i < gNumLights; i++) {
        currentLight = Lights[i];
        currentLight.Diffuse *= currentLight.Intensity;
        switch (currentLight.LightType) {
            case 0:
                float3 l = ComputeDirectionalLight(currentLight, mat, albedo, pixelIn, V, NdotV, F0, metallicValue, roughnessValue, specularValue);
                if (i == 0) {
                    l *= CalcDirectionalLightShadow(currentLight, pixelIn);
                }
                color += l;
                break;
            case 1:
                color += ComputePointLight(currentLight, mat, albedo, pixelIn, V, NdotV, F0, metallicValue, roughnessValue, specularValue);
                break;
        }
    }

    // IBL
    float3 R = reflect(-V, pixelIn.Normal);

    float3 prefilteredColor = CubeMap.SampleLevel(samLinearWrap, R, roughnessValue * MIPMAP_LEVEL_MAX).rgb;
    float3 irradiance = CubeMap.SampleLevel(samLinearWrap, pixelIn.Normal, MIPMAP_LEVEL_MAX).rgb;
    float2 brdf = TextureMaps[BRDFLutTextureIdx].Sample(samLinearWrap, float2(max(dot(pixelIn.Normal, -V), 0), roughnessValue));
    
    float3 kS = FresnelSchlick(F0, max(NdotV, 0.0), roughnessValue);
    float3 kD = (float3(1.0, 1.0, 1.0) - kS) * (float3(1.0, 1.0, 1.0) - metallicValue);

    float3 diffuseIBL = irradiance * albedo.rgb * mat.Diffuse.rgb * kD;

    float3 specularIBL = prefilteredColor * (kS * brdf.x + brdf.y);
    //float3 specularIBL = prefilteredColor * kS;

    float3 ambientIBL = diffuseIBL + specularIBL;
    color += ambientIBL / acos(-1);
    //color = color / (color + float3(1.0, 1.0, 1.0));

    color = ACESToneMap(color);
    color = pow(color, 1.0 / 2.2);

    return float4(color, albedo.a);
}