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

    // Diffuse
    vec3 lightDir = normalize(lightPosition - pos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * material.diffuse;

    // Specular (Blinn-Phong)
    float specularStrength = material.specular.r;
    vec3 viewDir = normalize(cameraPos - pos);
    vec3 halfDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(viewDir, halfDir), 0.0), material.shininess);
    vec3 specular = specularStrength * spec * lightColor;  

    // Combine lighting and texture
    vec4 lighting = vec4(ambient + diffuse + specular, material.opacity);
    vec4 texColor = texture(uTexture, uv);

    outColor = lighting * texColor;
}