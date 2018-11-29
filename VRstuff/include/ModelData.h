#ifndef PAKKI_MODEL_DATA
#define PAKKI_MODEL_DATA
#include <MathUtil.h>
#include <stdint.h>
#include <Utils.h>
#include <array>


#define MAX_NAME_SIZE 80
#define MAX_BONES 64
#define MAX_BONES_IN_VERTEX 3
struct BoneIndexes
{
	int	indexes[MAX_BONES_IN_VERTEX];
};
struct MeshData
{
	MATH::vec3*		vertexes = 0;
	MATH::vec3*		normals = 0;
	MATH::vec2*		texCoords = 0;
	int*			indexes = 0;
	BoneIndexes*    bonesIds;
	MATH::vec3*		weights;	
	//next model in line
};

struct meshAligment
{
	int numVerts = 0;
	int numTextureCoords = 0;
	int numNormals = 0;
	int numIndexes = 0;
	int numBoneData = 0;
};
#if 0
struct ModelData
{
	uint32_t numMeshes;
};
#endif
struct BoneData
{
	uint 		nodeIndex;
	MATH::mat4 	offset;
	MATH::mat4 	finalTransform;
};

struct AnimationData
{
	double duration;
	double ticksPerSecond;
	uint   numChannels;
	uint   channelsIndex;
};

struct RotationKey
{
	double 				time;
	MATH::quaternion	quat;
};

struct PositionKey
{
	double 				time;
	MATH::vec3			position;
};

struct ScaleKey
{
	double 				time;
	MATH::vec3			scale;
};


struct AnimationChannel
{
	uint nodeIndex;
	uint numRotationKeys;
	uint numPositionKeys;
	uint numScaleKeys;
	RotationKey* rotations = NULL;
	PositionKey* positions = NULL;
	ScaleKey*	scales = NULL;
};
#define NO_MESH 0xFFFF
#define NO_BONE 0xFFFF
#define NO_ANIME 0xFFFF
struct RenderNode
{
	uint 		numChildren;
	uint 		meshIndex = NO_MESH;
	uint 		boneIndex = NO_BONE;
	MATH::mat4 	transformation;
};

struct VertexBoneData
{
	std::array<uint,MAX_BONES_IN_VERTEX> IDs;
	std::array<float,MAX_BONES_IN_VERTEX> weights;
	void add(uint index, float weight)
	{
		for(uint i = 0; i < MAX_BONES_IN_VERTEX;i++)
		{
			if(weights[i] == 0.0)
			{
				weights[i] = weight;
				IDs[i] = index;
				break;
			}
		}
	 }
};


#endif
