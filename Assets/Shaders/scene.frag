#version 430 core
#include "PassData.glsl"

uniform sampler2DArray mainTexArray; // unit 2
uniform sampler2DShadow shadowTex;   // unit 1

in Vertex
{
    vec4 colour;
    vec2 texCoord;
    vec4 shadowProj;
    vec3 normal;
    vec3 worldPos;
    flat uint flags;
    flat uint materialID;
} IN;

out vec4 fragColor;

void main(void)
{
    uint matID = IN.materialID;
    bool hasTexture = (materials[matID].flags & 1u) != 0u;

    float shadow = 1.0;
    if (IN.shadowProj.w > 0.0) {
        shadow = textureProj(shadowTex, IN.shadowProj) * 0.5f;
    }

    vec3 incident = normalize(lightPosRadius.xyz - IN.worldPos);
    float lambert = max(0.0, dot(incident, IN.normal)) * 0.9;

    vec3 viewDir = normalize(cameraPos.xyz - IN.worldPos);
    vec3 halfDir = normalize(incident + viewDir);

    float rFactor = max(0.0, dot(halfDir, IN.normal));
    float sFactor = pow(rFactor, 80.0);

    vec4 albedo = IN.colour;
    if (hasTexture) {
        uint layer = materials[matID].texIndex;
        albedo *= texture(mainTexArray, vec3(IN.texCoord, float(layer)));
    }

    albedo.rgb = pow(albedo.rgb, vec3(2.2));

    fragColor.rgb  = albedo.rgb * 0.05f;
    fragColor.rgb += albedo.rgb * lightColor.rgb * lambert * shadow;
    fragColor.rgb += lightColor.rgb * sFactor * shadow;

    fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2f));
    fragColor.a   = albedo.a;
}