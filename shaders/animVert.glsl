#version 330 core
#define VERTEX

#include "shaders/AnimMeshData.inc"

out DATA
{
	vec2 uv;
	vec3 normal;
	vec3 fragPos;
	vec3 viewPos;
} vs_out;


void main()
{
	vs_out.fragPos = vec3(model * vec4(vertexPosition, 1.0));
	vec4 pos = calculate_glPos();
	gl_Position = pos;
	vs_out.uv = vec2(uv.x ,  uv.y);
	vs_out.normal = calculate_normal();

	mat4 inv = inverse(view);
	vs_out.viewPos = vec3(inv[3][0], inv[3][1], inv[3][2]);
}




