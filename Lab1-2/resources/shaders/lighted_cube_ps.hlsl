#include "resources/LightFunc.h"

struct GeomBuffer
{
    float4x4 model;
    float4x4 modelNormal;
    float4 params; // x - shininess, y - use normal map, z - texture index, w - is cube visible
};

cbuffer GeomBufferInst : register(b1)
{
    GeomBuffer instances[20];
}

Texture2DArray colorTexture : register(t0);
Texture2D normalTexture : register(t1);

SamplerState colorSampler : register(s0);

struct VSOutput
{
    float4 pos : SV_Position;
    float4 worldPos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
    nointerpolation uint id : INST_ID;
};

float4 PS(VSOutput input) : SV_Target0
{
    float4 resultColor = float4(0, 0, 0, 0);
    float3 objectColor = colorTexture.Sample(colorSampler, float3(input.uv, instances[input.id].params.z)).xyz;
    
    float3 normal = normalize(input.normal);
    if (instances[input.id].params.y > 0.0)
    {
        normal = normalTexture.Sample(colorSampler, input.uv).xyz * 2.0 - float3(1.0, 1.0, 1.0);
        float3 binorm = normalize(cross(input.normal, input.tangent));
        normal = normal.x * normalize(input.tangent) + normal.y * binorm + normal.z * normalize(input.normal);
    }
    
    resultColor = float4(CalcLight(objectColor, normal, input.worldPos.xyz, instances[input.id].params.x, false), 1);

    return resultColor;
}