#version 460
#include "common.glsl"

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) flat in int materialIdIn;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 norm = normalize(normal);
    Material material = materials[materialIdIn];
    
    vec3 ambient = material.ambient * lightColor;

    float distance = length(lightPosition - pos);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

    // Diffuse
    vec3 lightDir = normalize(lightPosition - pos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * material.diffuse;

    vec3 globalLightDirNorm = normalize(globalLightDir);
    float globalDiff = max(dot(norm, -globalLightDirNorm), 0.0);
    vec3 globalDiffuse = globalDiff * globalLightColor * material.diffuse;

    vec3 totalDiffuse = diffuse * attenuation + globalDiffuse;

    // Specular (Blinn-Phong)
    float specularStrength = material.specular.r;
    vec3 viewDir = normalize(cameraPos - pos);
    vec3 halfDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(viewDir, halfDir), 0.0), material.shininess);
    vec3 specular = specularStrength * spec * lightColor * attenuation;

    vec3 globalHalfDir = normalize(-globalLightDirNorm + viewDir);
    float globalSpec = pow(max(dot(norm, globalHalfDir), 0.0), material.shininess);
    vec3 globalSpecular = material.specular * globalSpec * globalLightColor;

    // Combine lighting and texture
    vec4 lighting = vec4(ambient + totalDiffuse + specular + globalSpecular, material.opacity);
    vec4 texColor = texture(uTexture, uv);

    outColor = lighting * texColor;
}