#ifndef PASS_DATA_GLSL
#define PASS_DATA_GLSL

#include "BindingSlots.glsl"

layout(std140, binding = PASS_UBO_SLOT) uniform PassData
{
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 viewProjMatrix;
	mat4 shadowMatrix;

	vec4 lightColor;
	vec4 lightPosRadius;

	vec4 cameraPos;
	vec4 misc; // time/mode/flags
};

#endif