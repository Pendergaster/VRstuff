#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Containers.h>
#include <gameDefs.h>
#include "game.h"

#include <imgui/imgui.h>
#include <imgui/imgui.cpp>
#include <imgui/imgui_draw.cpp>
#include <imgui/imgui_widgets.cpp>
#include "sound.h"


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

typedef uint RenderDataHandle;
struct GameRender
{
	RenderDataHandle*	dataHandles = NULL;	
	int					dataHandleIndex = 0;
	uint*				freelist = NULL;	
	int					freeListIndex = 0;
	uint*				renderDataBackPointers = NULL;
};

typedef uint RenderHandle;

struct Camera 
{
	MATH::vec3			position;
	MATH::vec3			direction;
	MATH::vec3			up;
	float				yaw;
	float				pitch;
};

#define MAX_RENDER_OBJECTS 100
static RenderData create_new_renderdata(uint materialID, uint meshID,
		const MATH::vec3& pos,const MATH::quaternion q,MATH::vec3 scale)
{
	RenderData ret;
	ret.materialID = materialID;
	ret.meshID = meshID;
	ret.position = pos;
	ret.scale = scale;
	ret.orientation = q;
	return ret;
}
static RenderData* get_render_data(RenderHandle handle,const GameRender& gameRend,
		RenderData* data)
{
	return  &data[gameRend.dataHandles[handle]];
}

static RenderHandle insert_renderdata(const RenderData& data,GameRender* render,RenderData* renderData)
{
	RenderDataHandle ret = 0;
	int numRenderables = render->dataHandleIndex - render->freeListIndex;
	renderData[numRenderables]= data;
	//always push to the back
	if(render->freeListIndex != 0)
	{
		--render->freeListIndex;
		ret = render->freelist[render->freeListIndex];
	}
	else
	{
		ASSERT_MESSAGE((render->dataHandleIndex + 1) < MAX_PLAYERS,"RenderData overflow \n");
		ret = numRenderables;
		render->dataHandleIndex++;
	}
	//because we are always iserting to back
	render->renderDataBackPointers[numRenderables] = ret;
	render->dataHandles[ret] = numRenderables;
	return ret;
}
static void dispose_renderData(RenderHandle handle,GameRender* render,RenderData* renderData)
{
	ASSERT_MESSAGE((render->freeListIndex + 1) < MAX_PLAYERS,"freelist overflow \n");

	uint numRenderObjects = render->dataHandleIndex - render->freeListIndex;

	if( 1 < numRenderObjects )
	{

		//swap actual data last one and handles id
		uint actualID = render->dataHandles[handle];
		uint lastIndex = numRenderObjects - 1;

		RenderData temp = renderData[actualID];
		renderData[actualID] = renderData[lastIndex];
		renderData[lastIndex] = temp;

		uint tempBackpointer = render->renderDataBackPointers[lastIndex];
		render->renderDataBackPointers[lastIndex] = render->renderDataBackPointers[actualID];
		render->renderDataBackPointers[actualID] = tempBackpointer;

		uint swapHandle = render->renderDataBackPointers[render->dataHandles[handle]]; // 2
		render->dataHandles[swapHandle] = render->dataHandles[handle] ;// 0	


		render->freelist[render->freeListIndex] = handle;// add 0 to back of the list 
		render->freeListIndex++;
	}
	else
	{
		render->freelist[render->freeListIndex] = handle;// add 0 to back of the list 
		render->freeListIndex++;
	}
}

static const MATH::vec3 worldUp(0.f,1.f,0.f);
static void init_camera(Camera* cam,MATH::mat4* view,MATH::mat4* projection,
		MATH::vec3 pos, MATH::vec3 at)
{
	MATH::identify(view);
	MATH::vec3 temp;
	cam->position = pos;
	//MATH::mat4 
	MATH::create_lookat_mat4(view,cam->position,at,worldUp);
	cam->up = worldUp;
	cam->yaw = -90.0f;
	cam->pitch = 0;

	MATH::vec3 cameraDirection = MATH::normalized(at -cam->position);
	cam->direction = cameraDirection;
	const float fov = 45.f;
#if !VR
	MATH::perspective(projection,MATH::deg_to_rad * fov, 
			(float)SCREENWIDHT / (float) SCREENHEIGHT,0.1f, 10000.f);
#else 
	MATH::perspective(projection,MATH::deg_to_rad * fov, 
			(float)(SCREENWIDHT/2) / (float) SCREENHEIGHT,0.1f, 10000.f);
#endif
}

struct Game
{
	GameRender			renderData;
	SoundContext		soundContext;
	Camera				camera;
	RenderDataHandle	planet;
};

EXPORT void init_game(void* p)
{
	//Game game;
	GameHook* hook = (GameHook*)p;
	Game* game = (Game*)CONTAINER::get_next_memory_block(hook->gameMemory);//(Game*)hook->userData;
	memset(game,0,sizeof(Game));
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(Game));
	hook->userData = game;
	hook->materials = (Material*)CONTAINER::get_next_memory_block(hook->gameMemory);
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(Material) * MaxMaterials);
	hook->numMaterials = MaxMaterials;

	hook->renderables = (RenderData*)CONTAINER::get_next_memory_block(hook->gameMemory);
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(RenderData) * MAX_RENDER_OBJECTS);
	hook->numRenderables = 0;

	//hook->renderIndexes = (int*)CONTAINER::get_next_memory_block(hook->gameMemory);
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(int) * MAX_RENDER_OBJECTS);

	game->renderData.dataHandles = (RenderDataHandle*)CONTAINER::get_next_memory_block(hook->gameMemory);
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(int) * MAX_RENDER_OBJECTS);
	game->renderData.dataHandleIndex = 0;

	game->renderData.dataHandles = (RenderDataHandle*)CONTAINER::get_next_memory_block(hook->gameMemory);
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(int) * MAX_RENDER_OBJECTS);
	game->renderData.dataHandleIndex = 0;

	game->renderData.freelist = (uint*)CONTAINER::get_next_memory_block(hook->gameMemory);
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(int) * MAX_RENDER_OBJECTS);
	game->renderData.freeListIndex = 0;

	game->renderData.renderDataBackPointers = (uint*)CONTAINER::get_next_memory_block(hook->gameMemory);
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(uint) * MAX_RENDER_OBJECTS);
	game->renderData.freeListIndex = 0;

	Material planetMat = create_new_material(hook->shaders,"MainProg");

	RenderData planetData = create_new_renderdata
		(
		 MaterialType::PlanetMat,
		 get_mesh(hook->meshes,"Planet"),
		 MATH::vec3(0.f,0,0),
		 MATH::quaternion(),
		 MATH::vec3(0.5f,0.5f,0.5f)
		);


	game->planet = insert_renderdata(planetData,
			&game->renderData,hook->renderables);

	planetData.position = MATH::vec3(0,-3.f,4.f);
	insert_renderdata(planetData,&game->renderData,hook->renderables);



	hook->numRenderables = 2;
	TextureID moonTex = get_texture(*hook->textures,"MoonTexture");
	set_material_texture(hook->shaders,&planetMat,0,moonTex);
	hook->materials[0] = planetMat;
	init_sound_device(&game->soundContext,&hook->workingMemory,&hook->gameMemory);

	init_camera(&game->camera,&hook->viewMatrix,&hook->projectionMatrix,
			MATH::vec3(0,0,6.f),MATH::vec3(0,0,5.f));


	ImGui::SetCurrentContext(hook->imguiContext);
	set_input_context(&hook->inputs);
}

EXPORT void on_game_reload(void* p)
{
	GameHook* hook = (GameHook*)p;
	Game* game = (Game*)hook->userData;
	set_sound_context(&game->soundContext);
	ImGui::SetCurrentContext(hook->imguiContext);
	set_input_context(&hook->inputs);
}

static void update_camera(Camera* cam,MATH::mat4* view)
{
	MATH::vec2 movement = get_mouse_movement();

	float sensitivity = 0.05f;
	scale(&movement,sensitivity);

	printf("%.3f %.3f \n",cam->yaw, cam->pitch);
	cam->yaw   -= movement.x;
	cam->pitch += movement.y;  

	if(cam->pitch > 89.0f)
		cam->pitch =  89.0f;
	if(cam->pitch < -89.0f)
		cam->pitch = -89.0f;

	cam->direction.x = cosf(MATH::deg_to_rad * cam->yaw) * 
		cosf(MATH::deg_to_rad * cam->pitch);
	cam->direction.y = sinf(MATH::deg_to_rad * cam->pitch);
	cam->direction.z = sinf(MATH::deg_to_rad * cam->yaw) * 
		cosf(MATH::deg_to_rad * cam->pitch);
	MATH::normalize(&cam->direction);

	cam->up = MATH::cross_product(cam->direction, worldUp);
	MATH::normalize(&cam->up);
	cam->up = MATH::cross_product(cam->up, cam->direction);
	MATH::normalize(&cam->up);
	//	normalize_vec3(&c->camUp);
	//	cross_product(&c->camUp, &c->camUp, &c->cameraDir);
	//	normalize_vec3(&c->camUp);

	printf("%.3f %.3f %.3f \n",cam->up.x,cam->up.y,cam->up.z);
	MATH::create_lookat_mat4(view,cam->position,
			cam->position + cam->direction, cam->up);
}



EXPORT void update_game(void* p)
{
	GameHook* hook = (GameHook*)p;
	Game* game = (Game*)hook->userData;
	ImGui::Begin("Game window ");
	ImGui::Text("lenght of song is %.3f \n",0.f);
	static bool pause = true;
	ImGui::Checkbox("pause",&pause);
	ImGui::End();

	float cameraSpeed = 4.f * 1.f/60.f; // adjust accordingly
	if (key_down(Key::KEY_W))
	{
		//printf("%.2f  %.2f  %.2f \n ",game->camera.direction.x, game->camera.direction.x, 
		//	game->camera.direction.x);
		game->camera.position += game->camera.direction * cameraSpeed;
	}
	if (key_down(Key::KEY_S))
	{
		game->camera.position -= game->camera.direction * cameraSpeed;
	}
	if (key_down(Key::KEY_A))
	{
		game->camera.position  -= MATH::normalized(MATH::cross_product(game->camera.direction,
					game->camera.up)) * cameraSpeed;
	}
	if (key_down(Key::KEY_D))
	{
		game->camera.position  += MATH::normalized(MATH::cross_product(game->camera.direction,
					game->camera.up)) * cameraSpeed;
	}

	update_camera(&game->camera,&hook->viewMatrix);
}

EXPORT void dispose_game(void* p)
{
	GameHook* hook = (GameHook*)p;
	Game* game = (Game*)hook->userData;
	alcMakeContextCurrent(NULL);
	alcDestroyContext(game->soundContext.context);
	alcCloseDevice(game->soundContext.device);
}




