#ifndef PAKKI_MESHDEFS
#define PAKKI_MESHDEFS
#include <Utils.h>
#include <Containers.h>

typedef int MeshId;
struct Mesh
{
	uint vertBuffer = 0;
	uint normalBuffer = 0;
	uint texCoordBuffer = 0;
	uint indexBuffer = 0;
	uint vao = 0;
};
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
	CONTAINER::StringTable<int>		meshCache;
	Mesh*							meshArray = NULL;
	MeshInfo*						meshInfos = NULL;
};

static MeshId get_mesh(MeshData* meshes,const char* name)
{
	int* index = CONTAINER::access_table(meshes->meshCache,name);
	ASSERT_MESSAGE(index,"MESH NOT FOUND :: %s \n",name);
	return *index;
}


#endif //PAKKI_MESHDEFS
