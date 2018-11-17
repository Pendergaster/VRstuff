#version 330 core

#include "shaders/PakkiMeshData.inc"

out DATA
{
	vec2 uv;
	vec3 normal;
	vec3 fragPos;
	vec3 viewPos;
    vec4 fragPosLightSpace;
} vs_out;


void main()
{
	vs_out.fragPos = vec3(model * vec4(vertexPosition, 1.0));
	
	vec4 pos = projection * view * model * vec4(vertexPosition, 1.0);
    gl_Position = pos;
	vs_out.uv = vec2(uv.x , 1 - uv.y);
	//vs_out.normal = normalize(normal); //TODO transpose inverse juttu pojalle
	vs_out.normal = normalize(mat3(transpose(inverse(model))) * normal);  
	mat4 inv = inverse(view);
	vs_out.viewPos = vec3(inv[3][0], inv[3][1], inv[3][2]);
	vs_out.fragPosLightSpace = shadowMatrix * vec4(vs_out.fragPos, 1.0);
}




