#version 330 core
in DATA
{
	vec2 uv;
	vec3 normal;
	vec3 WorldSpacePos;
} frag_in;

uniform vec4 color;
out vec4 _color;

void main()
{
	_color = vec4(color.x + frag_in.uv.x +frag_in.normal.x,frag_in.WorldSpacePos.x + color.y , frag_in.uv.y,frag_in.normal.x);
}
