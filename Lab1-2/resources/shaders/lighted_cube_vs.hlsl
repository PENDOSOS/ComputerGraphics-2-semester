cbuffer SceneBuffer : register(b0)
{
    float4x4 vp;
    float4 lightPos;
    float4 lightColor;
    float4 ambientColor;
    float3 cameraPos;
};

cbuffer GeomBuffer : register(b1)
{
    float4x4 model;
};

struct VSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

VSOutput VS(VSInput vertex)
{
    VSOutput result;

    result.pos = mul(vp, mul(model, float4(vertex.pos, 1.0)));
    result.normal = float4(vertex.normal, 1.0);
    result.tangent = float4(vertex.tangent, 1.0);
    result.uv = vertex.uv;

    return result;
}