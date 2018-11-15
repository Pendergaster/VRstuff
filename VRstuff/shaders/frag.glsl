#version 330 core

#include "shaders/GlobalLight.inc"
#include "shaders/shadowData.inc"

in DATA
{
	vec2 uv;
	vec3 normal;
	vec3 fragPos;
	vec3 viewPos;
    vec4 fragPosLightSpace;
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

	float shadow = shadow_calculation(frag_in.fragPosLightSpace,
		shadowMap);

	_color = vec4(vec3(lightColor.x ,lightColor.y ,lightColor.z) * shadow,1);
}
