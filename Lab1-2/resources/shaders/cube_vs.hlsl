#include "resources/SceneBuffer.h"

cbuffer GeomBuffer : register(b1)
{
    float4x4 model;
};

struct VSInput
{
    float3 pos : POSITION;
    float4 color : COLOR;
    float3 alpha : ALPHA;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float4 worldPos : POSITION;
    float4 color : COLOR;
};

VSOutput VS(VSInput input)
{
    VSOutput result;
    result.pos = mul(vp, mul(model, float4(input.pos, 1.0)));
    result.worldPos = mul(model, float4(input.pos, 1.0));
    result.color = float4(input.color.xyz, input.alpha.x);

    return result;
}