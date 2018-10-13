#version 330 core

#include "shaders/GlobalLight.inc"

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
	vec3 lightColor = CalculateGlobalLighting(
		frag_in.normal,
		viewDir,
		vec3(texture(tex,frag_in.uv)),
		vec3(0.0007f,0.7f,0.00007f),
		32.f);

	_color = vec4(lightColor.x ,lightColor.y     ,lightColor.z  ,1);
}
