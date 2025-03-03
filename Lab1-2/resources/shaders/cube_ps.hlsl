struct VSOutput
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

float4 PS(VSOutput pixel) : SV_Target0
{
    return pixel.color;
}