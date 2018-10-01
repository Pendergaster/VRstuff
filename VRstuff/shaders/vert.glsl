#version 330 core

#include "shaders/PakkiMeshData.inc"

out DATA
{
	vec2 uv;
	vec3 normal;
	vec3 fragPos;
} vs_out;


void main()
{
	vs_out.fragPos = vec3(model * vec4(vertexPosition, 1.0));
	vec4 pos = projection * view * model * vec4(vertexPosition, 1.0);
    gl_Position = pos;
	vs_out.uv = vec2(uv.x , 1 - uv.y);
	vs_out.normal = normal; //TODO transpose inverse juttu pojalle
}




