#ifndef PAKKI_MODELQUERY
#define PAKKI_MODELQUERY
#include <MathUtil.h>
#include "meshes.h"
#include <Utils.h>

#if 0
static void bone_transform(ModelCache* models,uint modelId,float time,MATH::mat4* matrixes)
{
	ModelInfo* model = &models->modelInfos[modelId];
	MATH::mat4 identity;
	MATH::identify(&identity);
	RenderNode* nodes = &models->renderNodes[model->renderNodesLoc];
	//todo finr affecting node anim!

}
#endif

static inline int find_channel(AnimationChannel* channels,int low ,int high,uint key)
{
#if 0
	for(uint i = 0; i < high; i++)
	{
		if(key == channels[i].nodeIndex) return i;
	}
	return -1;
#else
	if(high < low){
		return -1;
	}
	int mid =  (low+high) > 0 ?(low+high)/2 : 0;

	if(key == channels[mid].nodeIndex) {
		return mid;
	}
	if(key > channels[mid].nodeIndex) {
		return find_channel(channels,mid + 1, high,key);
	}
	return find_channel(channels,low, mid - 1,key);
#endif
}

static MATH::mat4 calculate_lerp_scale(float time,AnimationChannel* node)
{
	ASSERT_MESSAGE(node->numScaleKeys,"NO SCALE KEYS");
	MATH::vec3 scale;
	if(node->numScaleKeys == 1)
	{
		scale =  node->scales[0].scale;
	}
	else
	{
		uint frameIndex = 0;
		for(uint i = 0; i < node->numScaleKeys - 1;i++)
		{
			if(time < (float)node->scales[i + 1].time)
			{
				frameIndex = i;
				break;
			}
		}
		ScaleKey currentFrame = node->scales[frameIndex];
		ScaleKey nextFrame = node->scales[(frameIndex + 1) % node->numScaleKeys];

		float delta = (time - (float)currentFrame.time) /
			(float)(nextFrame.time - currentFrame.time);
		scale = currentFrame.scale + delta * (nextFrame.scale - currentFrame.scale);
	}

	MATH::mat4 m;

	MATH::identify(&m);
	m.mat[0][0] = scale.x;
	m.mat[1][1] = scale.y;
	m.mat[2][2] = scale.z;
	return m;
}

static MATH::mat4 calculate_lerp_rotation(float time,AnimationChannel* node)
{
	ASSERT_MESSAGE(node->numRotationKeys,"NO SCALE KEYS");
	MATH::quaternion q;
	if(node->numScaleKeys == 1)
	{
		q =  node->rotations[0].quat;
	}
	else
	{
		uint frameIndex = 0;
		for(uint i = 0; i < node->numRotationKeys - 1;i++)
		{
			if(time < (float)node->rotations[i + 1].time)
			{
				frameIndex = i;
				break;
			}
		}
		RotationKey currentFrame = node->rotations[frameIndex];
		RotationKey nextFrame = node->rotations[(frameIndex + 1) % node->numRotationKeys];

		float delta = (time - (float)currentFrame.time) /
			(float)(nextFrame.time - currentFrame.time);

		q = MATH::interpolate_q(currentFrame.quat,nextFrame.quat,delta);
		MATH::normalize(&q);
	}
	return MATH::mat4(q);
}

static MATH::mat4 calculate_lerp_position(float time,AnimationChannel* node)
{
	ASSERT_MESSAGE(node->numPositionKeys,"NO SCALE KEYS");
	MATH::vec3 translation;
	if(node->numPositionKeys == 1)
	{
		translation =  node->positions[0].position;
	}
	else
	{
		uint frameIndex = 0;
		for(uint i = 0; i < node->numPositionKeys - 1;i++)
		{
			if(time < (float)node->positions[i + 1].time)
			{
				frameIndex = i;
				break;
			}
		}
		PositionKey currentFrame = node->positions[frameIndex];
		PositionKey nextFrame = node->positions[(frameIndex + 1) % node->numPositionKeys];

		float delta = (time - (float)currentFrame.time) /
			(float)(nextFrame.time - currentFrame.time);

		translation = (currentFrame.position + delta * (nextFrame.position - currentFrame.position));
	}
	MATH::mat4 m;
	MATH::identify(&m);
	MATH::translate(&m,translation);
	return m;
}

static uint load_bones_from_nodes(RenderNode* nodes,uint depth,float time,
		const MATH::mat4 inverse,MATH::mat4 parentTransform,
		BoneData* bones,MATH::mat4* results,AnimationChannel* channels,
		uint numChannels)
{
	RenderNode* current = &nodes[depth];

	MATH::mat4 nodeTranform = current->transformation;

	int affectingChannel = find_channel(channels,0,numChannels,depth);
	if(affectingChannel != -1)
	{
		MATH::mat4 scaling = calculate_lerp_scale(time,&channels[affectingChannel]);
		MATH::mat4 rotation = calculate_lerp_rotation(time,&channels[affectingChannel]);
		MATH::mat4 position = calculate_lerp_position(time,&channels[affectingChannel]);
		nodeTranform = position * rotation * scaling;
	}
	MATH::mat4 globalTransform = parentTransform * nodeTranform;
	if(current->boneIndex != NO_BONE)
	{
		results[current->boneIndex] = inverse * globalTransform *  bones[current->boneIndex].offset;
	}
	for(uint i = 0; i < current->numChildren;i++)
	{
		depth = load_bones_from_nodes(nodes,depth+1,time,inverse,globalTransform,bones,results,channels,numChannels);
	}
	return depth;
}

static uint load_meshes_from_nodes(RenderNode* nodes,uint depth,float time,
		const MATH::mat4 inverse,MATH::mat4 parentTransform,MATH::mat4* meshTransforms)
{
	RenderNode* current = &nodes[depth];
	MATH::mat4 nodeTranform = current->transformation;


	MATH::mat4 globalTransform = parentTransform * nodeTranform;
	if(current->meshIndex != NO_MESH)
	{
		meshTransforms[current->meshIndex] =  inverse * globalTransform;
	}

	for(uint i = 0; i < current->numChildren;i++)
	{
		depth = load_meshes_from_nodes(nodes,depth+1,time,inverse,globalTransform,meshTransforms);
	}
	return depth;
}

static uint read_node_heirarchy(RenderNode* nodes,uint depth,
		const float time,AnimationChannel* channels,const uint numChannels,
		const MATH::mat4& parentTransform,MATH::mat4* boneResults,
		BoneData* bones,const MATH::mat4& inv,uint* meshIDs,MATH::mat4* meshTransforms,bool animated)
{
	RenderNode* current = &nodes[depth];
	MATH::mat4 nodeTranform = current->transformation;
	if(animated)
	{
		int affectingChannel = find_channel(channels,0,numChannels,depth);
		if(affectingChannel != -1)
		{
			MATH::mat4 scaling = calculate_lerp_scale(time,&channels[affectingChannel]);
			MATH::mat4 rotation = calculate_lerp_rotation(time,&channels[affectingChannel]);
			MATH::mat4 position = calculate_lerp_position(time,&channels[affectingChannel]);
			nodeTranform = position * rotation * scaling;	
		}
	}
	MATH::mat4 globalTransform = parentTransform * nodeTranform;
	if(current->boneIndex != NO_BONE && animated)
	{
		boneResults[current->boneIndex] = inv * globalTransform * bones[current->boneIndex].offset;
	}
	if(current->meshIndex != NO_MESH)
	{
		meshIDs[current->meshIndex] =  current->meshIndex;
		meshTransforms[current->meshIndex] =  inv * globalTransform;
	}
	for(uint i = 0; i < current->numChildren;i++)
	{
		depth = read_node_heirarchy(nodes,depth + 1,time,channels,
				numChannels,globalTransform,boneResults,bones,inv,meshIDs,meshTransforms,animated);
	}
	return depth;
}

#endif
