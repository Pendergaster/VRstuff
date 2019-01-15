

#define MAX_WEIGHTS 4
#define MAX_BONES 64
layout(std140) uniform MatrixBlock
{
	mat4 view;
	mat4 projection;
};
uniform mat4 bones[MAX_BONES];

uniform mat4 model;
layout 	(location = 0) in vec3 vertexPosition;
layout	(location = 2) in vec2 uv;
layout	(location = 1) in vec3 normal;
layout	(location = 3) in ivec4 boneIndexes;
layout	(location = 4) in vec4 weights;


mat4 finalBoneTrasform;

vec4 calculate_glPos()
{

	vec4 nW = normalize(weights);
	mat4 boneTransform = bones[boneIndexes[0]] * nW[0];
	boneTransform += 	 bones[boneIndexes[1]] * nW[1];
	boneTransform += 	 bones[boneIndexes[2]] * nW[2];
	boneTransform += 	 bones[boneIndexes[3]] * nW[3];	
	finalBoneTrasform = boneTransform;
	#if 0
		vec4 RE = boneTransform * vec4(1,1,1,1);
		vec4 ret;
		ret = projection * view * model * 
		(vec4(vertexPosition.xyz, 1.0) + (RE * 0.0001));

	#else
		vec4 ret;
		ret = projection * view * model * boneTransform *
		(vec4(vertexPosition.xyz, 1.0));

		#endif
		return ret;
}

vec3 calculate_normal()
{
	return normalize(mat3(inverse(transpose(model * finalBoneTrasform))) * normal);
}

