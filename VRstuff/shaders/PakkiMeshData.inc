
layout(std140) uniform MatrixBlock
{
	mat4 view;
	mat4 projection;
	mat4 shadowMatrix;
};
uniform mat4 model;
layout 	(location = 0) in vec3 vertexPosition;
layout	(location = 2) in vec2 uv;
layout	(location = 1) in vec3 normal;


