#ifndef PAKKI_MODELQUERY
#define PAKKI_MODELQUERY
#include <MathUtil.h>
#include "meshes.h"
#include <Utils.h>

static void bone_transform(ModelCache* models,uint modelId,float time,MATH::mat4* matrixes)
{
	ModelInfo* model = &models->modelInfos[modelId];
	MATH::mat4 identity;
	MATH::identify(&identity);
	RenderNode* nodes = &models->renderNodes[model->renderNodesLoc];
	//todo finr affecting node anim!
}

static inline int find_channel(AnimationChannel* channels,uint low ,uint high,uint key)
{

	if(high < low){
		return -1;
	}
	int mid = (low+high)/2;

	if(key == channels[mid].nodeIndex) {
		return mid;
	}
	if(key > channels[mid].nodeIndex) {
		find_channel(channels,mid + 1, high,key);
	}
	find_channel(channels,low, mid - 1,key);
}

static MATH::mat3 calculate_lerp_scale(float time,AnimationChannel* node)
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
// todo kato scaling!
}

static uint read_node_heirarchy(RenderNode* nodes,uint depth,
	const float time,AnimationChannel* channels,const uint numChannels,
	const MATH::mat4& parentTransform)
{
	RenderNode* current = &nodes[depth];
	int affectingChannel = find_channel(channels,0,numChannels,depth);

	if(affectingChannel != -1)
	{
		MATH::vec3 scaling;
		MATH::mat4 scalingM;
	}
	MATH::mat4 nodeTranform = current->transformation;

	MATH::mat4 globalTransform = parentTransform * nodeTranform;
	for(uint i = 0; i < current->numChildren;i++)
	{
		depth = read_node_heirarchy(nodes,depth + 1,time,channels,
				numChannels,globalTransform);
	}
	//if(current->boneIndex = )
	//	return depth;
}


#endif
