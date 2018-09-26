
layout(std140) uniform MatrixBlock
{
	mat4 view;
	mat4 projection;
};
uniform mat4 model;
layout 	(location = 0) in vec3 vertexPosition;
layout	(location = 1) in vec2 uv;
layout	(location = 2) in vec3 normal;


