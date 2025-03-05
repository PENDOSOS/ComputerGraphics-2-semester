cbuffer SceneBuffer : register(b0)
{
    float4x4 vp;
    float4 cameraPos;
};

cbuffer GeomBuffer : register(b1)
{
    float4x4 model;
};

struct VSInput
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

VSOutput VS(VSInput input)
{
    VSOutput result;

    result.pos = mul(vp, mul(model, float4(input.pos, 1.0)));
    result.color = input.color;

    return result;
}