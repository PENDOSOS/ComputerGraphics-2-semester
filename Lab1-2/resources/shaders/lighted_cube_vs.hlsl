#include "resources/SceneBuffer.h"

cbuffer GeomBuffer : register(b1)
{
    float4x4 model;
    float4x4 modelNormal;
    float4 params; // x - shininess, y - use normal map
};

struct VSInput
{
    float3 pos : POSITION;
    float3 tangent : TANGENT;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float4 worldPos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

VSOutput VS(VSInput vertex)
{
    VSOutput result;

    result.pos = mul(vp, mul(model, float4(vertex.pos, 1.0)));
    result.worldPos = mul(model, float4(vertex.pos, 1.0));
    result.normal = mul(modelNormal, float4(vertex.normal, 0.0)).xyz;
    result.tangent = mul(modelNormal, float4(vertex.tangent, 0.0)).xyz;
    result.uv = vertex.uv;

    return result;
}