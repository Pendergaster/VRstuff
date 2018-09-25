#version 330 core
layout 	(location = 0) in vec3 vertexPosition;
layout	(location = 1) in vec2 uv;
layout	(location = 2) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out DATA
{
	vec2 uv;
	vec3 normal;
	vec3 WorldSpacePos;
} vs_out;


void main()
{
    // note that we read the multiplication from right to left
	vec4 pos = projection * view * model * vec4(vertexPosition, 1.0);

    gl_Position = pos;// projection * view * model * vec4(vertexPosition, 1.0);
	vs_out.uv = vec2(uv.x , 1 - uv.y);
	vs_out.normal = normal;
	vs_out.WorldSpacePos = vec3(model * vec4(vertexPosition, 1.0));
}