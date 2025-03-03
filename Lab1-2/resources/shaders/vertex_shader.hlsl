struct VSIn
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VSOut
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

VSOut VS(VSIn input)
{
    VSOut result;
    
    result.pos = float4(input.pos, 1);
    result.color = input.color;
    
    return result;
}