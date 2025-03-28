#include "resources/SceneBuffer.h"

struct VSOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

Texture2D sceneTexture : register(t0);
SamplerState colorSampler : register(s0);

float4 PS(VSOut pixel) : SV_Target0
{
    float3 color = sceneTexture.Sample(colorSampler, pixel.uv).rgb;
    
    if (sceneParams.w > 0)
    {
        // za warudo filter
        float red = 1.0 - color.r;
        float green = 1.0 - color.g;
        float blue = 1.0 - color.b;
    
        color = float3(red, green, blue);
    }
    
    return float4(color, 1);
}