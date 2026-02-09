#version 430 core
#include "PassData.glsl"

layout(location = 0) in vec3 position;

out Vertex {
	vec3 viewDir;
} OUT;

void main(void)		{
	vec3 pos = position;
	mat4 invproj  = inverse(projMatrix);
	pos.xy	  *= vec2(invproj[0][0],invproj[1][1]);
	pos.z 	= -1.0f;

	OUT.viewDir		= transpose(mat3(viewMatrix)) * normalize(pos);
	gl_Position		= vec4(position, 1.0);
}