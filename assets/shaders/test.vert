#version 460
#include "common.glsl"

layout(location = 0) in vec3 posIn;
layout(location = 1) in vec3 normalIn;
layout(location = 2) in vec2 uvIn;

layout(location = 0) out vec3 posOut;
layout(location = 1) out vec3 normOut;
layout(location = 2) out vec2 uvOut;
layout(location = 3) out int materialIdOut;

void main() {
    mat4 model = instances[gl_InstanceID].model;
    int materialId = instances[gl_InstanceID].materialId;
    
    gl_Position = projection * view * model * vec4(posIn, 1.0);

    posOut = vec3(model * vec4(posIn, 1.0));
    normOut = mat3(transpose(inverse(model))) * normalIn;
    uvOut = uvIn;
    materialIdOut = materialId;
}