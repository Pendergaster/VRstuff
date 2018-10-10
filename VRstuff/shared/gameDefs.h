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
#define GAME_MEMORY_SIZE 100000
struct RenderData
{
	int					materialID;
	MeshId				meshID = 0;
	MATH::vec3			position;
	MATH::vec3			oriTemp;
	MATH::quaternion	orientation;
	float				scale = 0;
};


struct GameHook
{
	MeshData*				meshes;
	ShaderManager*			shaders;
	int						numRenderables = 0;
	RenderData*				renderables = NULL;
	int*					renderIndexes = NULL;
	int						numMaterials = 0;
	Material*				materials = NULL;
	CONTAINER::MemoryBlock	gameMemory;
	ImGuiContext*			imguiContext;
	TextureData*			textures;
};


#endif
