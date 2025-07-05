struct Instance {
    mat4 model;
    int materialId;
};

struct Material {
    vec3 ambient;
    float shininess;
    vec3 specular;
    float opacity;
    vec3 diffuse;
};

layout(std140, binding = 0) uniform Constants {
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
    vec3 lightPosition;
    vec3 lightColor;
    float elapsedTime;
};

layout(binding = 1) uniform sampler2D uTexture;

layout(std430, binding = 2) buffer InstanceBuffer {
    Instance instances[];
};

layout(std430, binding = 3) buffer MaterialBuffer {
    Material materials[];
};