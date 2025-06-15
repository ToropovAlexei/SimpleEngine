#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uvIn;

layout(location = 0) out vec2 uvOut;

layout(std140, binding = 0) uniform Constants {
    mat4 model;
    mat4 view;
    mat4 projection;
    float elapsedTime;
};

layout(std430, binding = 2) buffer InstanceBuffer {
    mat4 models[];
};

void main() {
    mat4 model = models[gl_InstanceID];
    
    gl_Position = projection * view * model * vec4(pos, 1.0);
    uvOut = uvIn;
}