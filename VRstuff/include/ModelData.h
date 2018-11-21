#ifndef PAKKI_MODEL_DATA
#define PAKKI_MODEL_DATA
#include <MathUtil.h>
#include <intTypes.h>
#define MAX_NAME_SIZE 80

struct ModelData
{
	MATH::vec3*		vertexes = 0;
	MATH::vec3*		normals = 0;
	MATH::vec2*		texCoords = 0;
	int*			indexes = 0;
	uint32_t	    nextModel = 0;
	//next model in line
};

struct ModelAligment
{
	int numVerts = 0;
	int numTextureCoords = 0;
	int numNormals = 0;
	int numIndexes = 0;
};




#endif
