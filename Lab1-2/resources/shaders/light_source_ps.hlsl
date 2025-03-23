struct VSOut
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

float4 PS(VSOut pixel) : SV_Target0
{
    return float4(pixel.color.rgb, 1);
}