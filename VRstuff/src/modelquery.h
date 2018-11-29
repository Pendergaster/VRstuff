#ifndef PAKKI_MODELQUERY
#define PAKKI_MODELQUERY
#include <MathUtil.h>
#include "meshes.h"

static void bone_transform(ModelCache* models,uint modelId,float time,MATH::mat4* matrixes)
{
	ModelInfo* model = &models->modelInfos[modelId];
	MATH::mat4 identity;
	MATH::identify(&identity);
	RenderNode* nodes = &models->renderNodes[model->renderNodesLoc];
	//todo finr affecting node anim!
}

static uint read_node_heirarchy(RenderNode* nodes,uint depth,
		float time,RenderNode* node,MATH::mat4 parentTransform)
{
	RenderNode* current = &nodes[depth];
	if(current->boneIndex = )
	return depth;
}


#endif
