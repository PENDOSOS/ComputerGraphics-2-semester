struct Light
{
    float4 lightPos;
    float4 lightColor;
};

cbuffer SceneBuffer : register(b0)
{
    float4x4 vp;
    int4 sceneParams; // x - light count, y - use normal map, z - show normals, w - use filter
    Light lights[3];
    float4 ambientColor;
    float4 frustum[6];
    float3 cameraPos;
};