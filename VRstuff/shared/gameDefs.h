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
#define SCREENWIDHT 1080
#define SCREENHEIGHT 600
#define VR 0

#define NO_ANIMATION 0xFFFFFFFF
#define MAX_ANIMATIONS 50
struct RenderData
{
	int					materialID;
	ModelId				meshID = 0;
	MATH::vec3			position;
	MATH::vec3			oriTemp;
	MATH::quaternion	orientation;
	MATH::vec3			scale;
	uint				animationIndex = NO_ANIMATION;
};

struct AnimationHook
{
	uint	animtionIndex = 0;
	float	animationSpeed = 0;
	float	animationPercent = 0;
	ModelId	modelID = 0;
	void*	engineData = NULL;
};

struct GlobalLight
{
	MATH::vec4 dir;
	MATH::vec4 ambient;
	MATH::vec4 diffuse;
	MATH::vec4 specular;
};

struct GameHook
{
	ModelCache*				models;
	ShaderManager*			shaders;
	int						numRenderables = 0;
	RenderData*				renderables = NULL;
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
	struct GlobalLight		globalLight;
	AnimationHook*			animations;
	MATH::vec3 				controllerPosRight;
	MATH::quaternion 		controllerRotRight;
	MATH::vec3 				controllerPosLeft;
	MATH::quaternion 		controllerRotLeft;
	MATH::vec3 				camPos;
	MATH::vec2 				stick;
	bool 					jumpButton;
};

#endif
