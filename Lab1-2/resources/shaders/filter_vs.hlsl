struct VSIn
{
    uint id : SV_VertexID;
};

struct VSOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

VSOut VS(VSIn vertex)
{
    VSOut output;
    float4 pos = float4(0, 0, 0, 0);
    switch (vertex.id)
    {
        case 0:
            pos = float4(-1, -3, 0, 1);
            break;
        case 1:
            pos = float4(-1, 1, 0, 1);
            break;
        case 2:
            pos = float4(3, 1, 0, 1);
            break;
    }
    output.pos = pos;
    output.uv = float2(pos.x * 0.5 + 0.5, 0.5 - pos.y * 0.5);
    return output;
}