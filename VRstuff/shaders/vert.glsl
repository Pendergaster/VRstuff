#version 330 core
#define VERTEX

#include "shaders/PakkiMeshData.inc"

#include "shaders/shadowData.inc"

out DATA
{
	vec2 uv;
	vec3 normal;
	vec3 fragPos;
	vec3 viewPos;
	vec4 lightSpacePos[NUM_CASCADES];
	float clipPositions[NUM_CASCADES];
	float clipSpace;
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

#if 0
	for (int i = 0 ; i < NUM_CASCADES ; i++) 
	{
		vs_out.lightSpacePos[i] = cascadeMatrix[i] * vec4(vs_out.fragPos,1.0);
												 // vec4(vertexPosition, 1.0);
		vs_out.clipPositions[i] = clipPositions[i];
	}
#endif


	vs_out.lightSpacePos[0] = cascadeMatrix[0] * vec4(vs_out.fragPos,1.0); 
												//vec4(vertexPosition, 1.0);
	vs_out.lightSpacePos[1] = cascadeMatrix[1] *vec4(vs_out.fragPos,1.0);
												// vec4(vertexPosition, 1.0);
	vs_out.lightSpacePos[2] = cascadeMatrix[2] *vec4(vs_out.fragPos,1.0); 
												//vec4(vertexPosition, 1.0);
	vs_out.lightSpacePos[3] = cascadeMatrix[3] *vec4(vs_out.fragPos,1.0);
												// vec4(vertexPosition, 1.0);
	vs_out.clipPositions[0] = -clipPositions.x;
	vs_out.clipPositions[1] = -clipPositions.y;
	vs_out.clipPositions[2] = -clipPositions.z;
	vs_out.clipPositions[3] = -clipPositions.w;


	//vs_out.clipSpace = gl_Position.z;
	vs_out.clipSpace = (view * vec4(vertexPosition.xyz, 1.0)).z;
	//vs_out.fragPosLightSpace = shadowMatrix * vec4(vs_out.fragPos, 1.0);
}




