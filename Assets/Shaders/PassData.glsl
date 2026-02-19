#ifndef PASS_DATA_GLSL
#define PASS_DATA_GLSL

#include "../../CSC8503/BindingSlots.h"

// ---------- Pass UBO ----------
layout(std140, binding = PASS_UBO_SLOT) uniform PassData
{
    mat4 viewMatrix;
    mat4 projMatrix;
    mat4 viewProjMatrix;
    mat4 shadowMatrix;

    vec4 lightColor;
    vec4 lightPosRadius;

    vec4 cameraPos;
    float time;
};

// ---------- Object SSBO ----------
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

// ---------- Material SSBO ----------
struct MaterialData {
    uint flags;      // bit0: hasTexture
    uint texIndex;   // C1 暂时不用，占位
    uint pad0;
    uint pad1;
};

layout(std430, binding = MATERIAL_SSBO_SLOT) readonly buffer MaterialBuffer {
    MaterialData materials[];
};

#endif