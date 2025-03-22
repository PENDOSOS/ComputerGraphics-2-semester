cbuffer SceneBuffer : register(b0)
{
    float4x4 vp;
    float4 lightPos;
    float3 lightColor;
    float4 ambientColor;
    float3 cameraPos;
};