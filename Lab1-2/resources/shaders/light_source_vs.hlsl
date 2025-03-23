#include "resources/SceneBuffer.h"

cbuffer GeomBuffer
{
    float4x4 model;
    float4x4 normalMatrix;
    float4 param; // x - light index
};

struct VSIn
{
    float3 pos : POSITION;
};

struct VSOut
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

VSOut VS(VSIn vertex)
{
    VSOut result;
    result.pos = mul(vp, mul(model, float4(vertex.pos, 1.0)));
    result.color = float4(lights[int(param.x)].lightColor.rgb, 1);
    
    return result;
}