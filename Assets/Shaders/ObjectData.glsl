#ifndef OBJECT_DATA_GLSL
#define OBJECT_DATA_GLSL

struct ObjectData {
    mat4 modelMatrix;
    vec4 colour;
    uint materialID;
    uint flags;
    uint pad0;
    uint pad1;
};

layout(std430, binding = OBJECT_SSBO_SLOT) readonly buffer ObjectBuffer {
    ObjectData objects[];
};

#endif