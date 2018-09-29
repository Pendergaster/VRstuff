#version 330 core

#include "shaders/PakkiMeshData.inc"

out DATA
{
	vec2 uv;
} vs_out;


void main()
{
	vec4 pos = projection * view * model * vec4(vertexPosition, 1.0);
    gl_Position = pos;
	vs_out.uv = vec2(uv.x , 1 - uv.y);
}




