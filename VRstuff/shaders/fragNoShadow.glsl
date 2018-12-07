#version 330 core
#define FRAGMENT
#include "shaders/GlobalLight.inc"

#line 0

in DATA
{
	vec2 uv;
	vec3 normal;
	vec3 fragPos;
	vec3 viewPos;
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

	vec3 finalColor = lightColor[0] + lightColor[1] + lightColor[2];  
	_color = vec4(finalColor.x ,finalColor.y ,finalColor.z ,1);
	_color.w = 1;
}
