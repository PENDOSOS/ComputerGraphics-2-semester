#include "resources/LightFunc.h"

struct VSOutput
{
    float4 pos : SV_Position;
    float4 worldPos : POSITION;
    float4 color : COLOR;
};

float4 PS(VSOutput pixel) : SV_Target0
{
    return float4(CalcLight(pixel.color.rgb, float3(1, 0, 1), pixel.worldPos.xyz, 0, true), pixel.color.w);
}