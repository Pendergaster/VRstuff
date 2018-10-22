#ifndef GAME_DEFS
#define GAME_DEFS
#include <Utils.h>
#include <MathUtil.h>
#include <Containers.h>
#include <imgui/imgui.h>
#include "meshdefs.h"
#include "materialdefs.h"
#include "ShaderDefs.h"
#include "texturedefs.h"
#include "sharedinputs.h"
#define GAME_MEMORY_SIZE 100000
#define GAME_WORKING_MEMORY 60000000
#define SCREENWIDHT 1000
#define SCREENHEIGHT 1000

struct RenderData
{
	int					materialID;
	MeshId				meshID = 0;
	MATH::vec3			position;
	MATH::vec3			oriTemp;
	MATH::quaternion	orientation;
	MATH::vec3			scale;
};


struct GameHook
{
	MeshData*				meshes;
	ShaderManager*			shaders;
	int						numRenderables = 0;
	RenderData*				renderables = NULL;
	//int*					renderIndexes = NULL;
	int						numMaterials = 0;
	Material*				materials = NULL;
	CONTAINER::MemoryBlock	gameMemory;
	CONTAINER::MemoryBlock	workingMemory;
	ImGuiContext*			imguiContext = NULL;
	TextureData*			textures = NULL;
	void*					userData = 0;
	float					runTime = 0;
	Input					inputs;
	MATH::mat4				viewMatrix;
	MATH::mat4				projectionMatrix;
};


#endif
