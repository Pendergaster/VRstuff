#ifndef PAKKI_MESHDEFS
#define PAKKI_MESHDEFS
#include <Utils.h>
#include <Containers.h>
#include <ModelData.h>
typedef int MeshId;
struct Mesh
{
	uint vertBuffer = 0;
	uint normalBuffer = 0;
	uint texCoordBuffer = 0;
	uint indexBuffer = 0;
	uint vao = 0;
	uint numIndexes = 0;
};
#if 0
struct MeshInfo
{
	char*	name = NULL;
	char*	path = NULL;
	int		numVerts = 0;
	int		numIndexes = 0;
};
struct MeshData
{
	uint							numMeshes = 0;
	CONTAINER::StringTable<int>		meshCache; // vain kutsumanimet, boneille ja muille joku?
	Mesh*							meshArray = NULL;
	MeshInfo*						meshInfos = NULL;
};

static MeshId get_mesh(MeshData* meshes,const char* name)
{
	int* index = CONTAINER::access_table(meshes->meshCache,name);
	ASSERT_MESSAGE(index,"MESH NOT FOUND :: %s \n",name);
	return *index;
}
#else
// meshID reference
struct ModelInfo
{
	char*	name = NULL;
	char*	path = NULL;
	uint	numParts = 0;
	uint 	partsLoc = 0;
	uint    numMeshes = 0;
	uint 	meshLoc = 0;
};

struct MeshData
{
	uint							numMeshes = 0;
	uint 							numParts = 0;
	uint							numInfos = 0;
	CONTAINER::StringTable<int>		meshCache; // vain kutsumanimet, boneille ja muille joku?
	ModelInfo*						meshInfos = NULL;
	Mesh*							meshArray = NULL;
	MeshPart*						meshParts = NULL;
};

static MeshId get_mesh(MeshData* meshData,const char* name)
{
	int* index = CONTAINER::access_table(meshData->meshCache,name);
	ASSERT_MESSAGE(index,"MESH NOT FOUND :: %s \n",name);
	return *index;
}


#endif

#endif //PAKKI_MESHDEFS
