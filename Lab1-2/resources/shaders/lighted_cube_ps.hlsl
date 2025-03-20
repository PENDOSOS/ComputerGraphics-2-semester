cbuffer SceneBuffer : register(b0)
{
    float4x4 vp;
    float4 lightPos;
    float4 lightColor;
    float4 ambientColor;
    float3 cameraPos;
};

cbuffer LightingParams : register(b2)
{
    float4 params; // x - shininess
};

struct VSOutput
{
    float4 pos : SV_Position;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

float4 PS(VSOutput input) : SV_Target0
{
    
    return ambientColor * 0.27;
}