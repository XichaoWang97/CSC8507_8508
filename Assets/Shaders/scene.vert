#version 430 core
#include "PassData.glsl"

uniform uint baseObjectIndex;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;

out Vertex
{
    vec4 colour;
    vec2 texCoord;
    vec4 shadowProj;
    vec3 normal;
    vec3 worldPos;
    flat uint flags; // 把 flags 带到 frag
    flat uint materialID;
} OUT;

void main(void)
{
    ObjectData obj = objects[baseObjectIndex + gl_InstanceID];

    mat4 modelMatrix = obj.modelMatrix;
    vec4 objectColour = obj.colour;

    bool hasVertexColours = (obj.flags & 2u) != 0u; // bit1

    mat4 mvp = projMatrix * viewMatrix * modelMatrix;
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

    vec4 worldPos4 = modelMatrix * vec4(position, 1.0);
    OUT.shadowProj = shadowMatrix * worldPos4;

    OUT.worldPos = worldPos4.xyz;
    OUT.normal   = normalize(normalMatrix * normalize(normal));

    OUT.texCoord = texCoord;

    OUT.colour = objectColour;
    if (hasVertexColours) {
        OUT.colour = objectColour * colour;
    }

    OUT.flags = obj.flags;

    gl_Position = mvp * vec4(position, 1.0);
    OUT.materialID = obj.materialID;
}