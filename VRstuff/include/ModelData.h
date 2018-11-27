#ifndef PAKKI_MODEL_DATA
#define PAKKI_MODEL_DATA
#include <MathUtil.h>
#include <stdint.h>
#include <Utils.h>
#include <array>


#define MAX_NAME_SIZE 80
#define MAX_BONES 64
#define MAX_BONES_IN_VERTEX 3
struct meshData
{
	MATH::vec3*		vertexes = 0;
	MATH::vec3*		normals = 0;
	MATH::vec2*		texCoords = 0;
	int*			indexes = 0;
	int				boneIndexes[3];
	MATH::vec3		weights;	
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

struct ModelData
{
	uint32_t numMeshes;
};

struct MeshPart
{
	MATH::mat4 		localTranform;
	uint32_t 		meshIndex;
};

struct BoneData
{
	MATH::mat4 offset;
	MATH::mat4 finalTransform;
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
	double time;
	MATH::quaternion	quat;
};

struct PositionKey
{
	double time;
	MATH::vec3			position;
};

struct ScaleKey
{
	double time;
	MATH::vec3			scale;
};


struct ChannelKeys
	RotationKey rotation;
	PositionKey position;
	ScaleKey	scale;
};

struct AnimationChannel
{
	uint numRotationKeys;
	uint numPositionKeys;
	uint numScaleKeys;
	RotationKey* rotations = NULL;
	PositionKey* positions = NULL;
	ScaleKey*	scales = NULL;

};

struct VertexBoneData
{
	std::array<uint,MAX_BONES_IN_VERTEX> IDs;
	std::array<float,MAX_BONES_IN_VERTEX> weights;
	void add(uint index, float weight)
	{
		for(uint i = 0; i < MAX_BONES_IN_VERTEX;i++)
		{
			if(weights[i] == 0)
			{
				weights[i] = weight;
				IDs[i] = index;
			}
		}
	 }
};


#endif
