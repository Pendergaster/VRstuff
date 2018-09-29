#version 330 core
in DATA
{
	vec2 uv;
} frag_in;

uniform sampler2D tex;
out vec4 _color;

void main()
{
	_color = texture(tex,frag_in.uv);
}
