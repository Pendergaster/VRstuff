#VertexStart
#version 330 core
layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec2 uv;

out vec2 _uv;
void main()
{

    gl_Position = vec4(vertexPosition,0.0,1.0);
	_uv = uv;
}
#VertexEnd

********************************************************

#FragmentStart
#version 330 core

in vec2 _uv;
uniform sampler2D frametex;
out vec4 color;


const float gamma = 2.2;

void main()
{
	color = vec4(texture2D(frametex,_uv));
    color.rgb = pow(color.rgb, vec3(1.0/gamma));
	color.w = 1.f;
}

#FragmentEnd
