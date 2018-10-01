#version 330 core

#include "shaders/GlobalLight.inc"

in DATA
{
	vec2 uv;
	vec3 normal;
	vec3 fragPos;
} frag_in;

uniform sampler2D tex;
out vec4 _color;

void main()
{
	vec3 viewPos = vec3(0.f,0.f,6.f);
	vec3 viewDir = normalize(viewPos - frag_in.fragPos);
	vec3 lightColor = CalculateGlobalLighting(globalLight,frag_in.normal,viewDir,vec3(texture(tex,frag_in.uv)),
		vec3(0.7f,0.7f,0.7f),32.f);
	_color = vec4(lightColor.x,lightColor.y,lightColor.z,1);
}
