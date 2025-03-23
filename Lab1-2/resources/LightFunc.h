#include "resources/SceneBuffer.h"

float3 CalcLight(in float3 objectColor, in float3 normal, in float3 pos, in float shininess, in bool trans)
{
    if (sceneParams.z > 0.0)
    {
        return float3(normal * 0.5 + float3(0.5, 0.5, 0.5));
    }

    float3 resultColor = float3(0, 0, 0);
    float3 resultAmbientColor = ambientColor.xyz * objectColor;
    resultColor += resultAmbientColor;

    for (int i = 0; i < sceneParams.x; i++)
    {
        float3 lightDir = lights[i].lightPos.xyz - pos;
        if (trans && dot(normal, lightDir) < 0.0)
        {
            normal = -normal;
        }
        float lightDist = length(lightDir);
        lightDir /= lightDist;
        float attenuation = clamp(1 / pow(lightDist, 2), 0, 1);
        float3 resultDiffuseColor = attenuation * max(dot(normal, lightDir), 0) * lights[i].lightColor.rgb * objectColor;
        resultColor += resultDiffuseColor;

        float3 viewDir = normalize(pos - cameraPos);
        float3 reflectDir = reflect(lightDir, normal);
        float3 resultSpecularColor = objectColor * lights[i].lightColor.rgb * pow(max(dot(viewDir, reflectDir), 0), shininess);
        resultColor += resultSpecularColor;
    }

    return resultColor;
}