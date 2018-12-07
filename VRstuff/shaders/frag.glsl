#version 330 core
#define FRAGMENT
#include "shaders/GlobalLight.inc"

#include "shaders/shadowData.inc"

#line 0

in DATA
{
	vec2 uv;
	vec3 normal;
	vec3 fragPos;
	vec3 viewPos;
	vec4 lightSpacePos[NUM_CASCADES];
	float clipPositions[NUM_CASCADES];
	float clipSpace;
} frag_in;

uniform sampler2D tex;
out vec4 _color;

void main()
{
	vec3 viewPos = frag_in.viewPos;
	vec3 viewDir = normalize(viewPos - frag_in.fragPos);
	vec3 lightColor[3] = vec3[3](vec3(0,0,0),vec3(0,0,0),vec3(0,0,0));
	CalculateGlobalLighting(
			lightColor,
			frag_in.normal,
			viewDir,
			vec3(texture(tex,frag_in.uv)),
			vec3(0.0007f,0.7f,0.00007f),
			32.f);
	vec3 lightPos = vec3(-3.0f, 8.0f, -1.0f);
	vec3 lightDir = normalize(lightPos - frag_in.fragPos);



	float shadow = 0;
	float bias = calculate_shadow_bias(normalize( frag_in.normal),lightDir);
	int i = 0;
	vec4 cas = vec4(0,0,0,0);

	do{
		if (frag_in.clipSpace <= frag_in.clipPositions[0]) {
			shadow = shadow_calculation(frag_in.lightSpacePos[0],shadowSampler[0],bias);
			//cas = vec4(0.1, 0.0, 0.0, 0.0);
			break;}
		if (frag_in.clipSpace <= frag_in.clipPositions[1]) {
			shadow = shadow_calculation(frag_in.lightSpacePos[1],shadowSampler[1],bias);
			//cas = vec4(0.0, 0.1, 0.0, 0.0);
			break;}
		if (frag_in.clipSpace <= frag_in.clipPositions[2]) {
			shadow = shadow_calculation(frag_in.lightSpacePos[2],shadowSampler[2],bias);
			//cas = vec4(0.0, 0.0, 0.1, 0.0);
			break;}
		if (frag_in.clipSpace <= frag_in.clipPositions[3]) {
			shadow = shadow_calculation(frag_in.lightSpacePos[3],shadowSampler[3],bias);
			break;}
	}while(false);


	vec3 finalColor = lightColor[0] + lightColor[1]*(1 -shadow) + lightColor[2]*(1 -shadow);  
	_color = vec4(finalColor.x ,finalColor.y ,finalColor.z ,1) + cas;
	_color.w = 1;
}
