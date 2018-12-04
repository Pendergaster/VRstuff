#ifndef PAKKI_RENDERER
#define PAKKI_RENDERER
#define NOMINMAX
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gameDefs.h>
#include <glad/glad.h>
#include "ShaderDefs.h"
#include <Containers.h>
#include <ShaderDefs.h>
#include "SystemUniforms.h"
#include "modelquery.h"
#include <texturedefs.h>
#include <shader_utils.h>

struct FrameTexture
{
	uint texture;
	uint buffer;
	uint textureWidth;
	uint textureHeight;
	int	 attachments;
};

enum FrameBufferAttacment : int
{
	None = 1 << 0,
	Color = 1 << 1,
	Depth = 1 << 2,
	Multisample = 1 << 3
};


static inline FrameTexture create_depth_texture(uint width,uint height)
{
	FrameTexture ret;
	ret.attachments = FrameBufferAttacment::Depth;
	ret.textureHeight = height;
	ret.textureWidth = width;
	glGenFramebuffers(1, &ret.buffer);  
	glBindFramebuffer(GL_FRAMEBUFFER,ret.buffer);

	glGenTextures(1, &ret.texture);
	glBindTexture(GL_TEXTURE_2D, ret.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
			width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, ret.buffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ret.texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);  
	return ret;
}
static inline FrameTexture create_new_frameTexture(uint width,uint height,GLenum attachment,int type)
{
	FrameTexture ret;
	ret.textureWidth = width;
	ret.textureHeight = height;
	glGenFramebuffers(1,&ret.buffer);
	glBindFramebuffer(GL_FRAMEBUFFER,ret.buffer);
	glGenTextures(1,&ret.texture);
	GLenum texturetype = 0;
	texturetype = BIT_CHECK(type,FrameBufferAttacment::Multisample) ? 
		GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D; 
	glBindTexture(texturetype,ret.texture);
	if(BIT_CHECK(type,FrameBufferAttacment::Multisample))
	{
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,NULL);
	}
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glBindTexture(texturetype,0);
	glFramebufferTexture2D(GL_FRAMEBUFFER,attachment,texturetype,ret.texture,0);
	if(type != FrameBufferAttacment::Depth && BIT_CHECK(type,FrameBufferAttacment::Depth))
	{
		uint rbo = 0;
		glGenRenderbuffers(1,&rbo);
		glBindRenderbuffer(GL_RENDERBUFFER,rbo);
		//TODO tähän vr dimensiot
		if(BIT_CHECK(type,FrameBufferAttacment::Multisample))
		{
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4,
					GL_DEPTH24_STENCIL8, width, height);  
		}
		else
		{

			glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,
					ret.textureWidth,ret.textureHeight);
		}
		glBindRenderbuffer(GL_RENDERBUFFER,0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,
				GL_RENDERBUFFER,rbo);
	}
	if(!(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)){
		ABORT_MESSAGE("FAILED TO SET FRAMEBUFFER \n");
	}
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	ret.attachments = type;
	glCheckError();
	return ret;
}
static inline void set_and_clear_frameTexture(const FrameTexture& frameTex)
{
	glCheckError();
	glBindFramebuffer(GL_FRAMEBUFFER,frameTex.buffer);
	glViewport(0, 0, frameTex.textureWidth, frameTex.textureHeight);
	glClearColor(0.f,0.f,0.f,1.f);
	if(BIT_CHECK(frameTex.attachments,FrameBufferAttacment::Depth) && BIT_CHECK(frameTex.attachments,FrameBufferAttacment::Color)  ) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	else if ( FrameBufferAttacment::Depth == frameTex.attachments ) {
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	else if ( FrameBufferAttacment::Color == frameTex.attachments) {
		glClear(GL_COLOR_BUFFER_BIT);
	}
	else
	{
		ABORT_MESSAGE("Error with frametexturetype !!");
	}
	glCheckError();
}

static void inline blit_frameTexture(FrameTexture from,FrameTexture to)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, from.buffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, to.buffer);
	glBlitFramebuffer(0, 0, from.textureWidth, from.textureHeight, 0, 0,
			to.textureWidth, to.textureHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST); 

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glCheckError();
}

static void inline blit_frameTexture(FrameTexture from,uint to)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, from.buffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, to);
	glBlitFramebuffer(0, 0, from.textureWidth, from.textureHeight, 0, 0,
			SCREENWIDHT, SCREENHEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST); 
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glCheckError();
}



struct CascadeShadowData
{
	glm::mat4	shadowMatrixes[NUM_CASCADES];
	float		clipPositions[NUM_CASCADES];
};

struct RenderMeshData
{
	uint		matId;
	MATH::mat4*	bones;
	MATH::mat4*	modelOrientations;
	uint*		meshes;
	uint		numMeshes;
	uint		meshLoc;
	uint		numbones;
	bool		animated;
};

struct Renderer
{
	MATH::mat4			view;
	MATH::mat4			projection;
	Material			shadowMat;
	Material			skyMaterial;
	Material			postCanvasMaterial;
	struct GlobalLight  light;
	RenderData*			renderData;
	uint				numRenderables;
	SystemUniforms*		globalUniforms;
	Material*			materials;
	FrameTexture        offscreen;
	FrameTexture        postProcessCanvas;
	FrameTexture cascades[NUM_CASCADES];
	uint				skyvao;
	uint				canvasvao;
};

static void init_renderer(Renderer* pass,ShaderManager* shaders,TextureData* textures)
{
	pass->offscreen = create_new_frameTexture(SCREENWIDHT,
			SCREENHEIGHT, GL_COLOR_ATTACHMENT0, 
			FrameBufferAttacment::Color | FrameBufferAttacment::Depth 
			| FrameBufferAttacment::Multisample  );
	pass->postProcessCanvas = create_new_frameTexture(SCREENWIDHT,
			SCREENHEIGHT, GL_COLOR_ATTACHMENT0, 
			FrameBufferAttacment::Color );

	for(uint i = 0; i < NUM_CASCADES;i++)
	{
		pass->cascades[i] = create_depth_texture(4096,4096);
	}

	pass->shadowMat = create_new_material(shaders,"ShadowProg");
	pass->skyMaterial = create_new_material(shaders,"SkyProg");

	pass->postCanvasMaterial = create_new_material(shaders,"PostPro");

	float skyboxVertices[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};
	set_material_texture(shaders,&pass->skyMaterial,0,get_texture(*textures,"sky_box"));
	glGenVertexArrays(1,&pass->skyvao);
	uint skyvbo = 0;
	glGenBuffers(1,&skyvbo);
	glBindVertexArray(pass->skyvao);
	glBindBuffer(GL_ARRAY_BUFFER,pass->skyvao);
	glBindBuffer(GL_ARRAY_BUFFER,skyvbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, 
			GL_STATIC_DRAW);

	glCheckError();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0);
	glCheckError();

	glBindVertexArray(0);

	glCheckError();

	float canvasVerts[] = 
	{
		// positions          // colors           // texture coords
		1.f,  1.f,   1.0f, 1.0f,   // top right
		1.f, -1.f,   1.0f, 0.0f,   // bottom right
		-1.f, -1.f,  0.0f, 0.0f,   // bottom left
		-1.f,  1.f,  0.0f, 1.0f    // top left 
	};

	unsigned int canvasInds[] =
	{  // note that we start from 0!
		3, 1, 0,				// first triangle
		3, 2, 1					// second triangle
	}; 

	uint buffers[2];
	glGenBuffers(2,buffers);
	glGenVertexArrays(1,&pass->canvasvao);
	glBindVertexArray(pass->canvasvao);

	glBindBuffer(GL_ARRAY_BUFFER,buffers[0]);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,
			sizeof(float) * 4,(void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,
			sizeof(float) * 4,(void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBufferData(GL_ARRAY_BUFFER,sizeof(canvasVerts),NULL,GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(canvasVerts),canvasVerts);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(canvasInds), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(canvasInds), canvasInds);
	glBindVertexArray(0);

}


static CascadeShadowData cascade_shadow_render(RenderMeshData* renderData, Renderer* rend,
		const ShaderManager& shaders,FrameTexture cascadeTextures[NUM_CASCADES],
		ModelCache* models)
{
	/**shasha boy***/
	CascadeShadowData ret;
	static float cascadeSplitLambda = 0.550f;
	float nearS = 0.1f;
	float farS  = 75.f;
	float clipRange = farS - nearS;
	float minz = nearS;
	float maxz  = nearS + clipRange;
	float range = maxz - minz;
	float ratio = maxz / minz;


	float cascadeSplits[NUM_CASCADES];

	glm::mat4 lightViews[NUM_CASCADES];
	glm::mat4 shadowOrthos[NUM_CASCADES];
	float splitDepths[NUM_CASCADES];

	for(uint i = 0; i < NUM_CASCADES;i++)
	{
		float p = (i + 1) / (float)NUM_CASCADES;
		float log = minz * std::pow(ratio,p);
		float uniform = minz + range * p;
		float d = cascadeSplitLambda * (log - uniform) + uniform;
		cascadeSplits[i] = (d - nearS) / clipRange;
	}

	//orthos
	float lastSplitDist = 0;

	for(uint i = 0; i < NUM_CASCADES;i++)
	{
		float splitDist = cascadeSplits[i];
		glm::vec3 corners[8] = {
			glm::vec3(-1, 1, -1),
			glm::vec3( 1, 1, -1),
			glm::vec3( 1,-1, -1),
			glm::vec3(-1,-1, -1),
			glm::vec3(-1, 1,  1),
			glm::vec3( 1, 1,  1),
			glm::vec3( 1,-1,  1),
			glm::vec3(-1,-1,  1),
		};
		glm::mat4 invCam = glm::inverse((*(glm::mat4*)&rend->projection) *
				(*(glm::mat4*)&rend->view)  );
		//project corners to worldspace
		for(uint i2 = 0; i2 < 8; i2++)
		{
			glm::vec4 invCorner = invCam * glm::vec4(corners[i2],1.0f);
			corners[i2] = invCorner  /invCorner.w;
		}
		for(uint i2 = 0; i2 < 4; i2++)
		{
			glm::vec3 dist = corners[i2 + 4] - corners[i2];
			corners[i2 + 4] = corners[i2] + (dist * splitDist);
			corners[i2] = corners[i2] + (dist * lastSplitDist);
		}

		//frustum center
		glm::vec3 frustumCenter = glm::vec3(0.f);
		for(uint i2 = 0; i2 < 8; i2++)
		{
			frustumCenter += corners[i2];
		}
		frustumCenter /= 8.f;
		float radius = 0;
		for(uint i2 = 0; i2 < 8; i2++)
		{
			float dist = glm::length(corners[i2] - frustumCenter);
			radius = glm::max(radius,dist);
		}
		radius = std::ceil(radius * 16.f) / 16.f;
		glm::vec3 maxExtents = glm::vec3(radius);
		glm::vec3 minExtents = -maxExtents;
		glm::vec3 lDir = glm::normalize(*(glm::vec3*)&rend->light.dir);
		glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lDir * -minExtents.z,
				frustumCenter,
				glm::vec3(0,1.f,0));
		glm::mat4 lightOrthoMatrix = glm::ortho(
				minExtents.x,maxExtents.x,
				minExtents.y,maxExtents.y,
				0.f, maxExtents.z - minExtents.z);

		splitDepths[i] = (nearS + splitDist * clipRange) * -1.0f;
		lightViews[i] = lightViewMatrix;
		shadowOrthos[i] = lightOrthoMatrix;
		lastSplitDist = cascadeSplits[i];//splitDepths[i];
	}
	for(int i = 0; i < 4; i ++)
	{
		ret.clipPositions[i] = splitDepths[i];
	}
	//printf("%.3f %.3f %.3f %.3f \n",splitDepths[0],splitDepths[1],splitDepths[2],splitDepths[3]);
	struct bindableMatrix{
		glm::mat4 view,ortho;
	} bind;

	glCheckError();
	ShaderProgram* prog = 
		&shaders.shaderPrograms[rend->shadowMat.shaderProgram];
	uint glID = shaders.shaderProgramIds[rend->shadowMat.shaderProgram];
	glUseProgram(glID);
	ASSERT_MESSAGE(prog->uniforms[0].type == UniformType::MODEL,"SHADOW PROG INVALID UNIFORM");
	uint modelPos =  prog->uniforms[0].location;


	glCheckError();
	for(uint i = 0; i < NUM_CASCADES; i++)
	{
		set_and_clear_frameTexture(cascadeTextures[i]);
		bind.ortho = shadowOrthos[i]; 
		bind.view = lightViews[i]; 
		ret.shadowMatrixes[i] = bind.ortho * bind.view;

		glCheckError();
		glBindBuffer(GL_UNIFORM_BUFFER,
				rend->globalUniforms->matrixUniformBufferObject);
		glBufferSubData(GL_UNIFORM_BUFFER,
				0,sizeof(bindableMatrix), (void*)&bind);
		glBindBuffer(GL_UNIFORM_BUFFER,0);

		for(uint j = 0; j < rend->numRenderables; j++)
		{
			RenderMeshData* currentRenderData = 
				&renderData[j];
			//ModelInfo* currentModelInfo = 
			//	&meshes.meshInfos[currentRenderData->meshID];
			for(uint part = 0; part < currentRenderData->numMeshes;part++)
			{
				Mesh* currentMesh = &models->meshArray[currentRenderData->meshLoc];
				//MeshPart* currentPart = 
				//	&meshes.meshParts[currentModelInfo->partsLoc + part];
				glCheckError();
				glUniformMatrix4fv(modelPos, 1, 
						GL_FALSE, (GLfloat*)&currentRenderData->modelOrientations[part]);//.mat);
				glCheckError();
				//set mesh
				//Mesh* currentMesh = &meshes.meshArray[currentModelInfo->meshLoc +
				//	currentPart->meshIndex];
				glBindVertexArray(currentMesh->vao);

				glDrawElements(GL_TRIANGLES,
						currentMesh->numIndexes,	
						GL_UNSIGNED_INT,0);
				glCheckError();
			}
		}
	}
	return ret;
}

static RenderMeshData* query_models(RenderData* renderData,uint numRenderables,ModelCache* models,
		CONTAINER::MemoryBlock* workingMem)
{
	//RenderData* data;
	//MATH::mat4* 
	static float dt = 0; 
	dt += 0.02f;
	float animeTime = fmodf(dt,0.9f);

	RenderMeshData* renderDatas = (RenderMeshData*)CONTAINER::get_next_memory_block(*workingMem);
	CONTAINER::increase_memory_block(workingMem,sizeof(RenderMeshData) * numRenderables);


	for(uint i = 0; i < numRenderables;i++)
	{
		RenderData* currentData = &renderData[i];
		ModelInfo* modelInfo = &models->modelInfos[currentData->meshID];
		RenderNode* startNode = &models->renderNodes[modelInfo->renderNodesLoc];

		Animation* anime = &models->animations[modelInfo->animationLoc];
		AnimationChannel* animeChannels = anime->animationChannel;
		BoneData* boneDatas = &models->bones[modelInfo->boneLoc];
		uint numChannels  = anime->animData.numChannels;

		MATH::mat4* orientations = (MATH::mat4*)CONTAINER::get_next_memory_block(*workingMem);
		CONTAINER::increase_memory_block(workingMem,sizeof(MATH::mat4) * modelInfo->numMeshes);
		MATH::mat4* bones = (MATH::mat4*)CONTAINER::get_next_memory_block(*workingMem);
		CONTAINER::increase_memory_block(workingMem,sizeof(MATH::mat4) * modelInfo->numBones);


		MATH::mat4 parentTransform;
		MATH::identify(&parentTransform);

		uint depth = load_meshes_from_nodes(startNode,0,animeTime,
				modelInfo->inverseMatrix,parentTransform,orientations);
		depth  = load_bones_from_nodes(startNode,0,animeTime,
				modelInfo->inverseMatrix,parentTransform,boneDatas,bones,animeChannels,numChannels);

		for(uint m = 0; m < modelInfo->numMeshes;m++)
		{
			MATH::mat4 model(currentData->orientation);
			MATH::translate(&model,currentData->position);
			MATH::scale_mat4(&model,currentData->scale);
			orientations[m] =  /*orientations[m] **/ model;
		}
		RenderMeshData data;
		data.animated = true;
		data.numMeshes = modelInfo->numMeshes;
		data.numbones = modelInfo->numBones;
		data.bones = bones;
		data.modelOrientations = orientations;
		data.matId = currentData->materialID;
		data.meshLoc = modelInfo->meshLoc;
		renderDatas[i] = data;
	}
	return renderDatas;
}

static void render_meshes(Renderer* renderValues ,
		RenderMeshData* renderData,ModelCache* models,ShaderManager* shaders,TextureData* textures,
		FrameTexture cascadeTextures[NUM_CASCADES],CascadeShadowData* shadowBinds)
{
	glBindBuffer(GL_UNIFORM_BUFFER,renderValues->globalUniforms->matrixUniformBufferObject);
	//uniforms->matrixUniformBufferObject);
	glBufferSubData(GL_UNIFORM_BUFFER,
			0,sizeOfMatrixBlock,(void*)&renderValues->view);
	glBindBuffer(GL_UNIFORM_BUFFER,0);
	//global lighting
	glCheckError();
	glBindBuffer(GL_UNIFORM_BUFFER,renderValues->globalUniforms->globalLightBufferObject);
	glCheckError();
	glBufferSubData(GL_UNIFORM_BUFFER,0,sizeOfGlobalLightBlock,(void*)&renderValues->light);
	glCheckError();
	glBindBuffer(GL_UNIFORM_BUFFER,0);

	//shadowmatrixes and clipspaces
	glCheckError();
	glBindBuffer(GL_UNIFORM_BUFFER,renderValues->globalUniforms->shadowBlockBufferObject);
	glCheckError();
	glBufferSubData(GL_UNIFORM_BUFFER,0,sizeOfShadowBlock
			, shadowBinds->shadowMatrixes);
	glCheckError();
	glBindBuffer(GL_UNIFORM_BUFFER,0);
	glCheckError();

	for(int i = 0; i < NUM_CASCADES; i++)
	{
		glActiveTexture(GL_TEXTURE0 + SHADOW_MAP_INDEXES + i);
		glBindTexture(GL_TEXTURE_2D,
				cascadeTextures[i].texture);
	}

	for(uint i = 0; i < renderValues->numRenderables; i++)
	{
		//RenderData* currentRenderData = &re[currentIndex];
		RenderMeshData* currentData = &renderData[i];
		Material* currentMaterial = &renderValues->materials[currentData->matId];

		ShaderProgram* prog = &shaders->shaderPrograms[currentMaterial->shaderProgram];
		uint glID = shaders->shaderProgramIds[currentMaterial->shaderProgram];
		glUseProgram(glID);

		glCheckError();
		bool setModel = false;
		uint modelLoc = 0;
		bool setBones = false;
		uint boneloc = 0;
		for(uint i2 = 0; i2 < currentMaterial->numUniforms;i2++)
		{
			Uniform* uniToSet = &shaders->uniforms[currentMaterial->uniformIndex + i2];
			UniformInfo* info = &prog->uniforms[i2];

			switch(info->type)
			{
				case UniformType::VEC4:
					{
						glUniform4f(info->location,uniToSet->_vec4.x,
								uniToSet->_vec4.y,uniToSet->_vec4.z,uniToSet->_vec4.w);
					}break;
				case UniformType::MAT4:
					{
						glUniformMatrix4fv(info->location,1,GL_FALSE,
								(GLfloat*)uniToSet->_mat4.mat);
					}break;
				case UniformType::FLOATTYPE:
					{
						glUniform1f(info->location,uniToSet->_float);
					}break;
				case UniformType::INTTYPE:
					{
						glUniform1i(info->location,uniToSet->_int);
					}break;
				case UniformType::SAMPLER2D:
					{
						glActiveTexture(GL_TEXTURE0 + info->glTexLocation);
						glBindTexture(GL_TEXTURE_2D,
								textures->textureIds[uniToSet->_textureCacheId]);
					}break;
				case UniformType::MODEL:
					{
						//glUniformMatrix4fv(info->location, 1, GL_FALSE, (GLfloat*)&model);//.mat);
						setModel = true;
						modelLoc = info->location;
					}break;
				case UniformType::BONES:
					{
						//glUniformMatrix4fv(info->location, 1, GL_FALSE, (GLfloat*)&model);//.mat);
						setBones = true;
						boneloc = info->location;
					}break;

				case UniformType::SHADOW:
					{
						//glActiveTexture(GL_TEXTURE0 + info->glTexLocation);
						//glBindTexture(GL_TEXTURE_2D,
						//		commands.shadowMap);
					}break;
				case UniformType::INVALID: default:
					{
						ABORT_MESSAGE("INVALID UNIFORM TYPE /n");
					}break;
			}
		}
		glCheckError();
		//ModelInfo* currentModelInfo = &models->meshInfos[currentRenderData->meshID];
		for(uint part = 0; part < currentData->numMeshes;part++)
		{
			if(setModel)
			{
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, 
						(GLfloat*)&currentData->modelOrientations[part]);//.mat);
			}
			if(setBones)
			{
				MATH::mat4* bonesI = (MATH::mat4*)malloc(
					currentData->numbones* sizeof(MATH::mat4));
				defer{free(bonesI);};
				for(int ree = 0 ; ree < currentData->numbones;ree++)
				{
					MATH::identify(&bonesI[ree]);
				}
				glUniformMatrix4fv(boneloc, currentData->numbones, GL_FALSE, 
						//(GLfloat*)bonesI);
						(GLfloat*)currentData->bones);//.mat);
			}

			Mesh* currentMesh = &models->meshArray[currentData->meshLoc + part];
			glBindVertexArray(currentMesh->vao);
			glDrawElements(GL_TRIANGLES,currentMesh->numIndexes,
					GL_UNSIGNED_INT,0);


		}
	}

	//render skybox
	glActiveTexture(GL_TEXTURE0);
	glDepthFunc(GL_LEQUAL); 
	uint glID = shaders->shaderProgramIds[renderValues->skyMaterial.shaderProgram];
	glUseProgram(glID);
	glCheckError();
	// ... set view and projection matrix
	MATH::mat4 tempview = renderValues->view;
	tempview.mat[3][0] = 0;
	tempview.mat[3][1] = 0;
	tempview.mat[3][2] = 0;
	tempview.mat[3][3] = 1;

	glCheckError();
	SHADER::set_mat4_name(glID,"view",tempview.mat);
	SHADER::set_mat4_name(glID,"projection",renderValues->projection.mat);

	glCheckError();

	glBindVertexArray(renderValues->skyvao);
	glActiveTexture(GL_TEXTURE0 );

	glCheckError();
	Uniform* uniToSet = &shaders->uniforms[renderValues->skyMaterial.uniformIndex];
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures->textureIds[uniToSet->_textureCacheId]);

	glCheckError();
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glCheckError();
	glDepthFunc(GL_LESS);

	glCheckError();
	glUseProgram(0);
	glBindVertexArray(0);

}

static void render_pass(Renderer* renderValues ,ModelCache* models,ShaderManager* shaders, 
		TextureData* textures,CONTAINER::MemoryBlock* workingMem)
{
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);  
	glCullFace(GL_BACK);  




	CONTAINER::MemoryBlock currentState = *workingMem;
	defer{*workingMem = currentState;};
	RenderMeshData* renderData = query_models(renderValues->renderData,renderValues->numRenderables,
			models,workingMem);
	glCheckError();
	CascadeShadowData shadowData = cascade_shadow_render(renderData, renderValues,*shaders,
			renderValues->cascades,models);
	glCheckError();

	set_and_clear_frameTexture(renderValues->offscreen);
#if 1
	render_meshes(renderValues,renderData,models,shaders,textures,
			renderValues->cascades,&shadowData);
	glCheckError();
	blit_frameTexture(renderValues->offscreen,renderValues->postProcessCanvas);
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(shaders->shaderProgramIds[renderValues->postCanvasMaterial.shaderProgram]);
	SHADER::set_vec2_name( shaders->shaderProgramIds[renderValues->postCanvasMaterial.shaderProgram],"pos"
			,MATH::vec2(0,0));
	SHADER::set_vec2_name(shaders->shaderProgramIds[renderValues->postCanvasMaterial.shaderProgram],"scale"
			,MATH::vec2(1,1));
	glBindVertexArray(renderValues->canvasvao);
	glCheckError();
	glActiveTexture(GL_TEXTURE0);
	glCheckError();
	glBindTexture(GL_TEXTURE_2D,
			renderValues->postProcessCanvas.texture);
	glCheckError();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 

	glEnable(GL_DEPTH_TEST);

	blit_frameTexture(renderValues->postProcessCanvas,0);
#else
	blit_frameTexture(renderValues->offscreen,renderValues->cascades->texture);
#endif

}
#endif //PAKKI_RENDERER
