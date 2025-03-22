#include "resources/SceneBuffer.h"

float3 CalcLight(in float3 objectColor, in float3 normal, in float3 pos, in float shininess, in bool trans)
{
    float3 resultColor = float3(0, 0, 0);
    float3 resultAmbientColor = ambientColor.xyz * objectColor;

    float3 lightDir = lightPos.xyz - pos;
    float lightDist = length(lightDir);
    lightDir /= lightDist;
    float attenuation = clamp(1 / pow(lightDist, 2), 0, 1);
    float3 resultDiffuseColor = attenuation * max(dot(normal, lightDir), 0) * lightColor * objectColor;

    float3 viewDir = normalize(pos - cameraPos);
    float3 reflectDir = reflect(lightDir, normal);
    float3 resultSpecularColor = objectColor * lightColor * pow(max(dot(viewDir, reflectDir), 0), shininess);

    resultColor = resultAmbientColor + resultDiffuseColor + resultSpecularColor;

    return resultColor;
}