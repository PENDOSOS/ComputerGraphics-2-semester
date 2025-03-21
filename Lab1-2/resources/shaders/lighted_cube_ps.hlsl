cbuffer SceneBuffer : register(b0)
{
    float4x4 vp;
    float4 lightPos;
    float3 lightColor;
    float4 ambientColor;
    float3 cameraPos;
};

cbuffer LightingParams : register(b2)
{
    float4 params; // x - shininess
};

Texture2D colorTexture : register(t0);
Texture2D normalTexture : register(t1);

SamplerState colorSampler : register(s0);

struct VSOutput
{
    float4 pos : SV_Position;
    float4 worldPos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

float4 PS(VSOutput input) : SV_Target0
{
    float4 resultColor = float4(0, 0, 0, 0);
    float3 objectColor = colorTexture.Sample(colorSampler, input.uv).xyz;
    float3 binorm = normalize(cross(input.normal, input.tangent));
    float3 normal = normalTexture.Sample(colorSampler, input.uv).xyz * 2.0 - float3(1.0, 1.0, 1.0); //normalize(input.normal);
    normal = normal.x * normalize(input.tangent) + normal.y * binorm + normal.z * normalize(input.normal);
    
    float3 resultAmbientColor = ambientColor.xyz * objectColor;
    
    float3 lightDir = lightPos.xyz - input.worldPos.xyz;
    float lightDist = length(lightDir);
    lightDir /= lightDist;
    float attenuation = clamp(1 / pow(lightDist, 2), 0, 1);
    float3 resultDiffuseColor = attenuation * max(dot(normal, lightDir), 0) * lightColor * objectColor;
    
    float3 viewDir = normalize(input.worldPos.xyz - cameraPos);
    float3 reflectDir = reflect(lightDir, normal);
    float3 resultSpecularColor = objectColor * lightColor * pow(max(dot(viewDir, reflectDir), 0), params.x);
    
    resultColor = float4(resultAmbientColor + resultDiffuseColor + resultSpecularColor, 1);

    return resultColor;
}