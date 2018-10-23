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
enum class BeatPosition : int
{
	None = 0,
	Right,
	RightUp,
	RightDown,
	RightLeft,
	Left,
	LeftUp,
	LeftDown,
	Up,
	Down,
	UpDown,
	MaxPosition
};

struct BeatBlock
{
	uint renderDataId;
};

enum GameState : int
{
	Pause = 1 << 1,
	SongStarted = 2 << 1,
	BeforeSongTime = 3 << 1,
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
#define MAX_BEATS 20
struct BeatCube
{
	RenderHandle	rendData;
	float			startTime = 0;
};
struct HitCube
{
	RenderHandle	render;
	float			lerpValue;
};
struct Camera 
{
	MATH::vec3	position;
	MATH::vec3	direction;
	MATH::vec3	up;
	float		yaw;
	float		pitch;
};

struct Game
{
	GameRender			renderData;
	SoundContext		soundContext;
	BeatPosition*		positions;
	uint				numBeats;
	float				songRunTime = 0;
	float				beatRunTime = 0;
	int					state = Pause;
	PlayerHandle		song;
	float				songPitch;
	float				intervalFreq = 0;
	uint				currentInterval = 0;
	int					numActiveBeatCubes = 0;
	BeatCube*			beatCubes;
	HitCube				hitPositions[4];
	Camera				camera;
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

static RenderData* get_render_data(RenderHandle handle,const GameRender& gameRend,RenderData* data)
{
	return  &data[gameRend.dataHandles[handle]];
}
static RenderHandle insert_renderdata(const RenderData& data,GameRender* render,RenderData* renderData)
{
	//RenderData* insertHere = NULL;
	RenderDataHandle ret = 0;
	int numRenderables = render->dataHandleIndex - render->freeListIndex;
	renderData[numRenderables]= data;
	//always push to the back
	if(render->freeListIndex != 0)
	{
		--render->freeListIndex;
		ret = render->freelist[render->freeListIndex];
		//actualIndex = render->dataHandles[ret];
		//insertHere = &renderData[actualIndex];
		//index = *freelistIndex;
	}
	else
	{
		ASSERT_MESSAGE((render->dataHandleIndex + 1) < MAX_PLAYERS,"RenderData overflow \n");
		ret = numRenderables;//render->dataHandleIndex;
		//actualIndex = ret;
		//insertHere = &renderData[ret];
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
		//swap data handles
		//uint tempPointer = render->dataHandles[lastIndex];
		//render->dataHandles[handle] = ;
		//

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
	//who is pointing to last one?
}

static const MATH::vec3 worldUp(0.f,1.f,0.f);
static void init_camera(Camera* cam,MATH::mat4* view,MATH::mat4* projection,
		MATH::vec3 pos, MATH::vec3 at)
{
	MATH::identify(view);
	MATH::vec3 temp;
	cam->position = pos;
	MATH::vec3 target(0.f,0.f,0.f);
	cam->direction = MATH::vec3(0.f,0.f,-1.f);

	//MATH::mat4 
	MATH::create_lookat_mat4(view,&cam->position,&at,&worldUp);
	cam->up = worldUp;
	cam->yaw = -90.0f;
	cam->pitch = 0;

	//perspective(&projection, 
	//deg_to_rad(fov), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 10000.f);

	const float fov = 45.f;
#if VR
	MATH::perspective(projection,MATH::deg_to_rad * fov, 
			(float)SCREENWIDHT / (float) SCREENHEIGHT,0.1f, 10000.f);
#else 
	MATH::perspective(projection,MATH::deg_to_rad * fov, 
			(float)(SCREENWIDHT/2) / (float) SCREENHEIGHT,0.1f, 10000.f);
#endif
}



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

	game->beatCubes = (BeatCube*)CONTAINER::get_next_memory_block(hook->gameMemory);
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(BeatCube) * MAX_BEATS);
	game->numActiveBeatCubes = 0;




	//hook->materials[0] = 
	Material planetMat = create_new_material(hook->shaders,"MainProg");

	RenderData planetData = create_new_renderdata
		(
		 MaterialType::PlanetMat,
		 get_mesh(hook->meshes,"Cube"),
		 MATH::vec3(-3.f,0,0),
		 MATH::quaternion(),
		 MATH::vec3(0.5f,0.5f,0.5f)
		);

	//static RenderHandle insert_renderdata(
	//const RenderData& data,GameRender* render,RenderData* renderData)

	RenderDataHandle d1 = insert_renderdata(planetData,&game->renderData,hook->renderables);
	planetData.position.x += 2.f;
	RenderDataHandle d2 =  insert_renderdata(planetData,&game->renderData,hook->renderables);
	planetData.position.x += 2.f;
	RenderDataHandle d3 =  insert_renderdata(planetData,&game->renderData,hook->renderables);
	planetData.position.x += 2.f;
	RenderDataHandle d4 =  insert_renderdata(planetData,&game->renderData,hook->renderables);

	game->hitPositions[0].render = d1; 
	game->hitPositions[1].render = d2; 
	game->hitPositions[2].render = d3; 
	game->hitPositions[3].render = d4; 
	game->hitPositions[0].lerpValue = 1.f; 
	game->hitPositions[1].lerpValue = 1.f; 
	game->hitPositions[2].lerpValue = 1.f; 
	game->hitPositions[3].lerpValue = 1.f; 
	//static void dispose_renderData(RenderHandle handle,GameRender* render,RenderData* renderData,
	//uint numRenderObjects)

	hook->numRenderables = game->renderData.dataHandleIndex - game->renderData.freeListIndex;
	//printf("%d \n",hook->numRenderables);
	//static RenderData create_new_renderdata
	//(uint materialID, uint meshID,const MATH::vec3& pos,float scale)
#if 0
	planetData.materialID = MaterialType::PlanetMat;
	planetData.meshID = get_mesh(hook->meshes,"Planet");
	planetData.orientation = MATH::quaternion();
	planetData.position = MATH::vec3(0,0,0);
	planetData.scale = 0.5f;
	planetData.oriTemp = MATH::vec3();
#endif

	TextureID moonTex = get_texture(*hook->textures,"MoonTexture");
	set_material_texture(hook->shaders,&planetMat,0,moonTex);
	hook->materials[0] = planetMat;
	//hook->renderables[0] = planetData;
	//hook->renderIndexes[0] = 0;
	//hook->numRenderables = 1;

	//SoundContext sounds;
	//todo init working mem ymsyms
	init_sound_device(&game->soundContext,&hook->workingMemory,&hook->gameMemory);
	LOG("Sounds inited \n");
	//set_listener_worldData(MATH::vec3(0,0,6.f),MATH::vec3(),
	//		MATH::vec3(),MATH::vec3(0,1.f,0));
	LOG("pos set \n");
	PlayerHandle s = get_new_player();
	LOG("player set \n");
	set_player(s,SoundType::sound,MATH::vec3(),MATH::vec3(),1.f,1.f,true);
	LOG("Sounds inited \n");

	player_pause(s);
	game->song = s;
	ImGui::SetCurrentContext(hook->imguiContext);
	set_input_context(&hook->inputs);
	printf("Game INITED \n");

	const float bpm = 60.f;
	const float bps = bpm / 60.f;
	float lenght = get_sound_info(SoundType::sound).audioLenght;
	game->numBeats = (int)(lenght / bps);

	game->positions = (BeatPosition*)CONTAINER::get_next_memory_block(hook->gameMemory);
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(BeatPosition) * game->numBeats);

	uint randSeed = 10u;
	MATH::seed_rand(randSeed);
	for(BeatPosition* i = game->positions; i < game->positions + game->numBeats;i++)
	{
		*i = (BeatPosition)MATH::irand_range((int)BeatPosition::MaxPosition);
		//printf("%d ,\n",*i);
	}
	game->state = Pause;
	game->intervalFreq = 1.f / bps;
	game->songPitch = 1.f;

	init_camera(&game->camera,&hook->viewMatrix,&hook->projectionMatrix,
			MATH::vec3(0,4.f,6.f),MATH::vec3(0,0,0));
}

EXPORT void on_game_reload(void* p)
{
	GameHook* hook = (GameHook*)p;
	Game* game = (Game*)hook->userData;
	set_sound_context(&game->soundContext);
	ImGui::SetCurrentContext(hook->imguiContext);
	set_input_context(&hook->inputs);
}



EXPORT void update_game(void* p)
{
	GameHook* hook = (GameHook*)p;
	Game* game = (Game*)hook->userData;
	//RenderData* tempData = get_render_data(game->temp,game->renderData,hook->renderables);

	init_camera(&game->camera,&hook->viewMatrix,&hook->projectionMatrix,
			MATH::vec3(0,2.f,8.f),MATH::vec3(0,0,-3.f));



	MATH::quaternion rot(MATH::vec3(0,1,0),MATH::deg_to_rad * 50.f);
	static float dt = 0;
	dt += 1.f / 60.f;
	//hook->renderables->position.x = -sinf(dt/2.f); 	
	//hook->renderables->position.y = -sinf(dt); 	
	//hook->renderables->position.z = -sinf(dt/2.f) * 10; 	
	//tempData->oriTemp.y  += -2.f;
	//tempData->oriTemp.x  += 2.f;
	//set_player_pos(game->,hook->renderables->position);
	ImGui::Begin("Game window ");
	ImGui::Text("lenght of song is %.3f \n",g_soundContext->soundInfos->audioLenght);
	bool pause =BIT_CHECK(game->state,GameState::Pause);
	bool before = pause;
	ImGui::Checkbox("pause",&pause);

	if(pause != before)
	{
		if(BIT_CHECK(game->state , GameState::SongStarted))
		{
			if(pause)
			{
				player_pause(game->song);
			}	
			else
			{
				player_play(game->song);
			}

		}
		BIT_FLIP(game->state,GameState::Pause);
	}

	ImGui::Text("current runtime is %.3f \n",hook->runTime);
	static float gain = 0.f;
	ImGui::SliderFloat("gain",&gain,0.f,1.f);
	set_player_gain(game->song,gain);
	ImGui::SliderFloat("pitch",&game->songPitch,0.f,1.f);
	set_player_pitch(game->song,game->songPitch);
	set_listener_worldData(MATH::vec3(0,0,6.f),MATH::vec3(0,0,0),
			MATH::vec3(0,0,-1.f),MATH::vec3(0,1.f,0.f));

	const float travelTime = 4.0f;

	float rdt = 1.f / 60.f;
	float gameDT = rdt * game->songPitch;
	float startZ = -20.f;
	float endZ = 2.f;
	float gapSize = 2.f;
	float lerpIncrease = 3.f * gameDT;
	if(!BIT_CHECK(game->state,GameState::Pause))
	{
		const float minScale = 0.1f;
		const float  maxScale = 0.5f;

		{
			float tempInc = lerpIncrease;
			if(key_down(Key::KEY_W) || joy_key_down(JoyKey::KEY_ARROW_UP))
			{
				tempInc *= -2.f;
			}

			RenderData* c  = get_render_data(game->hitPositions[2].render
					,game->renderData,hook->renderables);
			game->hitPositions[2].lerpValue += tempInc;
			LIMIT(game->hitPositions[2].lerpValue,0,1.f);
			float amountScaled = c->scale.y;
			c->scale.y = MATH::lerp(minScale,maxScale,game->hitPositions[2].lerpValue);
			amountScaled -= c->scale.y;
			c->position.y = 0.4f; // up from ground
			c->position.y *=game->hitPositions[2].lerpValue;
		}
		{
			float tempInc = lerpIncrease;
			if(key_down(Key::KEY_A) || joy_key_down(JoyKey::KEY_ARROW_LEFT))
			{
				tempInc *= -2.f;
			}

			RenderData* c  = get_render_data(game->hitPositions[0].render
					,game->renderData,hook->renderables);
			game->hitPositions[0].lerpValue += tempInc;
			LIMIT(game->hitPositions[0].lerpValue,0,1.f);
			float amountScaled = c->scale.y;
			c->scale.y = MATH::lerp(minScale,maxScale,game->hitPositions[0].lerpValue);
			amountScaled -= c->scale.y;
			c->position.y = 0.4f; // up from ground
			c->position.y *=game->hitPositions[0].lerpValue;

		}
		{
			float tempInc = lerpIncrease;
			if(key_down(Key::KEY_S) || joy_key_down(JoyKey::KEY_ARROW_DOWN))
			{
				tempInc *= -2.f;
			}
			RenderData* c  = get_render_data(game->hitPositions[1].render
					,game->renderData,hook->renderables);
			game->hitPositions[1].lerpValue += tempInc;
			LIMIT(game->hitPositions[1].lerpValue,0,1.f);
			float amountScaled = c->scale.y;
			c->scale.y = MATH::lerp(minScale,maxScale,game->hitPositions[1].lerpValue);
			amountScaled -= c->scale.y;
			c->position.y = 0.4f; // up from ground
			c->position.y *=game->hitPositions[1].lerpValue;


		}
		{
			float tempInc = lerpIncrease;
			if(key_down(Key::KEY_D) || joy_key_down(JoyKey::KEY_ARROW_RIGHT))
			{
				tempInc *= -2.f;
			}
			RenderData* c  = get_render_data(game->hitPositions[3].render
					,game->renderData,hook->renderables);
			game->hitPositions[3].lerpValue += tempInc;
			LIMIT(game->hitPositions[3].lerpValue,0,1.f);
			float amountScaled = c->scale.y;
			c->scale.y = MATH::lerp(minScale,maxScale,game->hitPositions[3].lerpValue);
			amountScaled -= c->scale.y;
			c->position.y = 0.4f; // up from ground
			c->position.y *=game->hitPositions[3].lerpValue;


		}



		game->beatRunTime += gameDT;
		if(game->beatRunTime > get_sound_info(SoundType::sound).audioLenght + travelTime)
		{
			game->beatRunTime = 0;
			game->currentInterval = 0;
			printf("resetting the beat time \n");
			game->songRunTime = 0;
			wind_player_to(game->song,0.f);
			player_pause(game->song);
			BIT_UNSET(game->state,GameState::SongStarted);
			printf("Pausing the song! \n");
		}

		if(game->beatRunTime > travelTime )
		{
			if(!BIT_CHECK(game->state,GameState::SongStarted))
			{
				printf("song started! \n");
				BIT_SET(game->state,GameState::SongStarted);
				player_play(game->song);
			}
			game->songRunTime += gameDT;
		}
		uint newInterval = ((uint)(game->beatRunTime / game->intervalFreq));
		if(newInterval != game->currentInterval)
		{
			if(newInterval < game->numBeats)
			{
				game->currentInterval = newInterval;
				BeatPosition SpawnType = game->positions[newInterval];
				//printf("Spawning new cube! \n");
				float xs[2] = {0,0};
				//[l][d][u][r]
				if(SpawnType == BeatPosition::Down)
				{
					xs[0] = -gapSize / 2.f;
				}
				else if(SpawnType == BeatPosition::Left)
				{
					xs[0] = -gapSize / 2.f  -gapSize;
				}
				else if(SpawnType == BeatPosition::Right)
				{
					xs[0] = gapSize / 2.f + gapSize;
				}
				else if(SpawnType == BeatPosition::Up)
				{
					xs[0] = gapSize / 2.f;
				}
				RenderData planetData = create_new_renderdata(
						MaterialType::PlanetMat,
						get_mesh(hook->meshes,"Cube"),
						MATH::vec3(-3.f,2.f,0),
						MATH::quaternion(),
						MATH::vec3(0.5f,0.5f,0.5f));

				BeatCube cube;
				cube.startTime = game->beatRunTime;
				ASSERT_MESSAGE((game->numActiveBeatCubes + 2) < MAX_BEATS,"TOO MANY BEATS \n");
				if(xs[0])
				{
					planetData.position = MATH::vec3(xs[0],0.3f,startZ);
					cube.rendData = insert_renderdata(planetData,&game->renderData,hook->renderables);
					game->beatCubes[game->numActiveBeatCubes] = cube ;
					game->numActiveBeatCubes++;
				}
				if(xs[1])
				{
					planetData.position = MATH::vec3(xs[1],0.3f,startZ);
					cube.rendData = insert_renderdata(planetData,&game->renderData,hook->renderables);
					game->beatCubes[game->numActiveBeatCubes] = cube ;
					game->numActiveBeatCubes++;
				}
			}
		}

		for(int i = 0; i < game->numActiveBeatCubes;i++)
		{
			BeatCube* current = &game->beatCubes[i];
			float lifeTime =game->beatRunTime - current->startTime;
			if (lifeTime > travelTime)
			{
				dispose_renderData(current->rendData,&game->renderData,hook->renderables);
				if(i != game->numActiveBeatCubes - 1)
				{
					game->beatCubes[i] = game->beatCubes[game->numActiveBeatCubes -1];
					i--;
				}
				game->numActiveBeatCubes--;
			}
			else
			{
				float lerpVal = lifeTime / travelTime;
				get_render_data(current->rendData,game->renderData,hook->renderables)->position.z = MATH::lerp(startZ,endZ,lerpVal);
			}
		}
		//if()
	}

	ImGui::Text("current song time is %.3f \n",game->songRunTime);
	ImGui::Text("current game time %.3f \n",game->beatRunTime);
	ImGui::Text("max beats for song %d \n",game->numBeats);
	ImGui::Text("beats for song %d \n",game->numActiveBeatCubes);

	ImGui::End();

	hook->numRenderables = game->renderData.dataHandleIndex - game->renderData.freeListIndex;
	//printf(" update \n");
}
EXPORT void dispose_game(void* p)
{
	GameHook* hook = (GameHook*)p;
	Game* game = (Game*)hook->userData;
	alcMakeContextCurrent(NULL);
	alcDestroyContext(game->soundContext.context);
	alcCloseDevice(game->soundContext.device);
}
