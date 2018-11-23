#ifndef PAKKI_MODEL_DATA
#define PAKKI_MODEL_DATA
#include <MathUtil.h>
#include <stdint.h>
#define MAX_NAME_SIZE 80

struct meshData
{
	MATH::vec3*		vertexes = 0;
	MATH::vec3*		normals = 0;
	MATH::vec2*		texCoords = 0;
	int*			indexes = 0;
	//next model in line
};

struct meshAligment
{
	int numVerts = 0;
	int numTextureCoords = 0;
	int numNormals = 0;
	int numIndexes = 0;
};

struct ModelData
{
	uint32_t numMeshes;
};

struct MeshPart
{
	MATH::mat4 		localTranform;
	uint32_t 		meshIndex;
};





#endif
