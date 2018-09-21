#ifndef PAKKI_MODEL_DATA
#define PAKKI_MODEL_DATA
#include <MathUtil.h>

struct ModelData
{
	MATH::vec3*		vertexes = 0;
	MATH::vec3*		normals = 0;
	MATH::vec2*		normalCoords = 0;
	int*			index = 0;
};

struct ModelAligment
{
	int numVerts = 0;
	int numTextureCoords = 0;
	int numNormals = 0;
	int numIndexes = 0;
};









#endif
