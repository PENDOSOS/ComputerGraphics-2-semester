#include "resources/LightFunc.h"

cbuffer GeomBuffer : register(b1)
{
    float4x4 model;
    float4x4 modelNormal;
    float4 params; // x - shininess, y - use normal map
};

Texture2D colorTexture : register(t0);
Texture2D normalTexture : register(t1);

SamplerState colorSampler : register(s0);

struct VSOutput
{
    float4 pos : SV_Position;
    float4 worldPos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

float4 PS(VSOutput input) : SV_Target0
{
    float4 resultColor = float4(0, 0, 0, 0);
    float3 objectColor = colorTexture.Sample(colorSampler, input.uv).xyz;
    
    float3 normal = normalize(input.normal);
    if (params.y > 0.0)
    {
        normal = normalTexture.Sample(colorSampler, input.uv).xyz * 2.0 - float3(1.0, 1.0, 1.0);
        float3 binorm = normalize(cross(input.normal, input.tangent));
        normal = normal.x * normalize(input.tangent) + normal.y * binorm + normal.z * normalize(input.normal);
    }
        
    float3 a = CalcLight(objectColor, normal, input.worldPos.xyz, params.x, false);
    
    resultColor = float4(a, 1);

    return resultColor;
}