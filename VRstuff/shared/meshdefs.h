#ifndef PAKKI_MESHDEFS
#define PAKKI_MESHDEFS
#include <Utils.h>
#include <Containers.h>
#include <ModelData.h>
typedef int ModelId;
struct Mesh
{
	uint vertBuffer = 0;
	uint normalBuffer = 0;
	uint texCoordBuffer = 0;
	uint indexBuffer = 0;
	uint vao = 0;
	uint numIndexes = 0;
	uint bonedataBuffer = 0;
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
	char*			name = NULL;
	char*			path = NULL;
	uint			numMeshes = 0;
	uint			numBones = 0;
	uint			meshLoc = 0;
	uint			renderNodesLoc = 0;
	uint			numAnimations = 0;
	uint			animationLoc = 0;
	uint			boneLoc;
	MATH::mat4		inverseMatrix;
};

struct Animation
{
	AnimationData     animData;
	AnimationChannel* animationChannel;
};

struct ModelCache
{
	uint 							numModels = 0;
	uint							numMeshes = 0;
	uint							numRenderNodes = 0;
	uint							numBones = 0;
	uint							numAnimationsChannels = 0;
	uint							numAnimations = 0;
	CONTAINER::StringTable<int>		modelCache; // vain kutsumanimet, boneille ja muille joku?
	ModelInfo*						modelInfos = NULL;
	Mesh*							meshArray = NULL;
	RenderNode*						renderNodes = NULL;
	AnimationChannel*				animationChannels = NULL;
	Animation*						animations = NULL;
	RotationKey*					rotationKeys = NULL;
	PositionKey*					positionKeys = NULL;
	ScaleKey*						scaleKeys = NULL;
	BoneData*						bones = NULL;
};

static ModelId get_model(ModelCache* meshData,const char* name)
{
	int* index = CONTAINER::access_table(meshData->modelCache,name);
	ASSERT_MESSAGE(index,"MESH NOT FOUND :: %s \n",name);
	return *index;
}


#endif

#endif //PAKKI_MESHDEFS
