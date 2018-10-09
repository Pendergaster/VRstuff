#include <stdio.h>
#include <Containers.h>
#include <gameDefs.h>
#include "game.h"

#define MATERIALS(MODE)\
	MODE(PlanetMat)\


enum MaterialType : int
{
	MATERIALS(GENERATE_ENUM)
		MaxMaterials
};

const char* material_names[] = 
{
	MATERIALS(GENERATE_STRING)
};



struct Game
{
	int*	renderablesFreeList = NULL;	
	int		renderablesFreelistSize = 0;
};
#define MAX_RENDER_OBJECTS 100
EXPORT void init_game(void* p)
{
	Game game;
	GameHook* hook = (GameHook*)p;
	hook->materials = (Material*)CONTAINER::get_next_memory_block(hook->gameMemory);
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(Material) * MaxMaterials);
	hook->numMaterials = MaxMaterials;

	hook->renderables = (RenderData*)CONTAINER::get_next_memory_block(hook->gameMemory);
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(RenderData) * MAX_RENDER_OBJECTS);
	hook->numRenderables = 0;

	hook->renderIndexes = (int*)CONTAINER::get_next_memory_block(hook->gameMemory);
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(int) * MAX_RENDER_OBJECTS);

	game.renderablesFreeList = (int*)CONTAINER::get_next_memory_block(hook->gameMemory);
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(int) * MAX_RENDER_OBJECTS);
	game.renderablesFreelistSize = 0;

	//hook->materials[0] = 
	Material planetMat = create_new_material(hook->shaders,"MainProg");
	hook->materials[0] = planetMat;
	
	RenderData planetData;
	planetData.materialID = MaterialType::PlanetMat;
	planetData.meshID = get_mesh(hook->meshes,"Planet");
	planetData.orientation = MATH::quaternion();
	planetData.position = MATH::vec3(0,0,0);
	planetData.scale = 0.5f;
	planetData.oriTemp = MATH::vec3();

	hook->renderables[0] = planetData;
	hook->renderIndexes[0] = 0;
	hook->numRenderables = 1;

	printf("Game INITED \n");
}

EXPORT void update_game(void* p)
{
	GameHook* hook = (GameHook*)p;
	printf(" update \n");
}
