#include "resources/SceneBuffer.h"

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

struct VSInput
{
    float3 pos : POSITION;
    float3 tangent : TANGENT;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    uint instId : SV_InstanceID;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float4 worldPos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
    nointerpolation uint id : INST_ID;
};

VSOutput VS(VSInput vertex)
{
    VSOutput result;

    result.pos = mul(vp, mul(instances[vertex.instId].model, float4(vertex.pos, 1.0)));
    result.worldPos = mul(instances[vertex.instId].model, float4(vertex.pos, 1.0));
    result.normal = mul(instances[vertex.instId].modelNormal, float4(vertex.normal, 0.0)).xyz;
    result.tangent = mul(instances[vertex.instId].modelNormal, float4(vertex.tangent, 0.0)).xyz;
    result.uv = vertex.uv;
    result.id = vertex.instId;

    return result;
}