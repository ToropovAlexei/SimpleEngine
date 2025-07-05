#version 460

struct Instance {
    mat4 model;
    int materialId;
};

layout(location = 0) in vec3 posIn;
layout(location = 1) in vec3 normalIn;
layout(location = 2) in vec2 uvIn;

layout(location = 0) out vec3 posOut;
layout(location = 1) out vec3 normOut;
layout(location = 2) out vec2 uvOut;
layout(location = 3) out int materialIdOut;

layout(std140, binding = 0) uniform Constants {
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
    vec3 lightPosition;
    vec3 lightColor;
    float elapsedTime;
};

layout(std430, binding = 2) buffer InstanceBuffer {
    Instance instances[];
};

void main() {
    mat4 model = instances[gl_InstanceID].model;
    int materialId = instances[gl_InstanceID].materialId;
    
    gl_Position = projection * view * model * vec4(posIn, 1.0);

    posOut = vec3(model * vec4(posIn, 1.0));
    normOut = mat3(transpose(inverse(model))) * normalIn;
    uvOut = uvIn;
    materialIdOut = materialId;
}