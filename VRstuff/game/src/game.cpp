
#define _ITERATOR_DEBUG_LEVEL 2
#define _CRT_SECURE_NO_WARNINGS
#define MIKA 0
#if MIKA 
#include <Raknet-Shared/ClientInformation.h>
#include <Raknet-Shared/MessageCodes.h>

#include <Raknet-Client/Client.h>
#include <Raknet-Client/Client.cpp>
#endif

#include <stdio.h>
#include <Containers.h>
#include <gameDefs.h>
#include "game.h"

#include <imgui/imgui.h>
#include <imgui/imgui.cpp>
#include <imgui/imgui_draw.cpp>
#include <imgui/imgui_widgets.cpp>
#include "sound.h"

//#include<bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
//#include<bullet/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
//#include<bullet/btBulletCollisionCommon.h>


#define MATERIALS(MODE)\
	MODE(PlanetMat)\
	MODE(Lattia)\
	MODE(Man)\


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

#define MAX_RENDER_OBJECTS 100
void init_game_renderer(GameRender* render,CONTAINER::MemoryBlock* mem)
{
	render->dataHandles = (RenderDataHandle*)CONTAINER::get_next_memory_block(*mem);
	CONTAINER::increase_memory_block(mem,sizeof(int) * MAX_RENDER_OBJECTS);
	render->dataHandleIndex = 0;

	render->dataHandles = (RenderDataHandle*)CONTAINER::get_next_memory_block(*mem);
	CONTAINER::increase_memory_block(mem,sizeof(int) * MAX_RENDER_OBJECTS);
	render->dataHandleIndex = 0;

	render->freelist = (uint*)CONTAINER::get_next_memory_block(*mem);
	CONTAINER::increase_memory_block(mem,sizeof(int) * MAX_RENDER_OBJECTS);
	render->freeListIndex = 0;

	render->renderDataBackPointers = (uint*)CONTAINER::get_next_memory_block(*mem);
	CONTAINER::increase_memory_block(mem,sizeof(uint) * MAX_RENDER_OBJECTS);
	render->freeListIndex = 0;
}

struct GameAnimations
{
	AnimationHook*	animations;
	uint*			animationFreeList;
	uint			animationIndex;
	uint			freelistIndex;
} *animations = NULL;

static void set_animation_context(GameAnimations* animes)
{
	ASSERT_MESSAGE(animations == NULL,"ANIMATION CONTEXT SET FAILED");
	animations = animes;
}

static void init_game_animations(GameAnimations* animes,CONTAINER::MemoryBlock* mem)
{
	animes->animations = (AnimationHook*)CONTAINER::get_next_memory_block(*mem);
	CONTAINER::increase_memory_block(mem,sizeof(AnimationHook) * MAX_ANIMATIONS);

	animes->animationFreeList = (uint*)CONTAINER::get_next_memory_block(*mem);
	CONTAINER::increase_memory_block(mem,sizeof(uint) * MAX_ANIMATIONS);
	animes->animationIndex = 0;
	animes->freelistIndex = 0;
	set_animation_context(animes);
}

typedef uint AnimeHandle;
static AnimeHandle create_new_animation(uint animationIndex,
		ModelId model,
		float animationSpeed = 1.f,
		float animationPerCent = 0)
{
	AnimationHook* data;
	uint index = 0;
	if(animations->freelistIndex != 0)
	{
		animations->animationIndex--;
		index = animations->animationFreeList[animations->animationIndex];
		data = &animations->animations[animations->animationFreeList[animations->animationIndex]];
	}
	else
	{
		data = &animations->animations[animations->animationIndex];
		index  = animations->animationIndex;
		animations->animationIndex++;
	}
	data->animtionIndex = animationIndex;
	data->animationPercent = animationPerCent;
	data->animationSpeed = animationSpeed;
	data->modelID = model;
	data->engineData = NULL;
	return index;
}

static void dispose_animation(AnimeHandle handle)
{
	animations->animationFreeList[animations->freelistIndex] = handle;
	animations->freelistIndex++;
}

typedef uint RenderHandle;

struct Camera 
{
	MATH::vec3			position;
	MATH::vec3			direction;
	MATH::vec3			up;
	float				yaw;
	float				pitch;
};

static RenderData create_new_renderdata(uint materialID, uint meshID,
		const MATH::vec3& pos,const MATH::quaternion q,MATH::vec3 scale)
{
	RenderData ret;
	ret.materialID = materialID;
	ret.meshID = meshID;
	ret.position = pos;
	ret.scale = scale;
	ret.orientation = q;
	ret.animationIndex = NO_ANIMATION;
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
	//always push to the bac
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
	cam->yaw = 40.0f;
	cam->pitch = 0;

	MATH::vec3 cameraDirection = MATH::normalized(at -cam->position);
	cam->direction = cameraDirection;
	const float fov = 90.f;
#if !VR
	MATH::perspective(projection,MATH::deg_to_rad * fov, 
			(float)SCREENWIDHT / (float) SCREENHEIGHT,0.1f, 75.f);
#else 
	MATH::perspective(projection,MATH::deg_to_rad * fov, 
			(float)(SCREENWIDHT/2) / (float) SCREENHEIGHT,0.1f, 75.f);
#endif
}

#define MAX_BOXES 100
struct PhysicsBox
{
	RenderDataHandle	boxRenderData;
	btRigidBody*		body;
};
struct Game
{
	GameRender			renderData;
	SoundContext		soundContext;
	Camera				camera;
	RenderDataHandle	planet;
	RenderDataHandle	enemy;
	GameAnimations      animations;
#if MIKA
	Client				client;
#endif
	btDiscreteDynamicsWorld*	dynamicsWorld;
	uint						numBoxes;
	PhysicsBox					boxes[MAX_BOXES];
};


EXPORT void init_game(void* p)
{
	//Game game;
	GameHook* hook = (GameHook*)p;
	//Game tempGame;
	Game* game = (Game*)CONTAINER::get_next_memory_block(hook->gameMemory);//(Game*)hook->userData;
	memset(game,0,sizeof(Game));
	//*game = tempGame;
#if MIKA
	new(&game->client)Client;
#endif
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(Game));
	hook->userData = game;
	hook->materials = (Material*)CONTAINER::get_next_memory_block(hook->gameMemory);
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(Material) * MaxMaterials);
	hook->numMaterials = MaxMaterials;

	hook->renderables = (RenderData*)CONTAINER::get_next_memory_block(hook->gameMemory);
	CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(RenderData) * MAX_RENDER_OBJECTS);
	hook->numRenderables = 0;

	init_game_renderer(&game->renderData,&hook->gameMemory);
	init_game_animations(&game->animations,&hook->gameMemory);
	hook->animations = game->animations.animations;
	//hook->renderIndexes = (int*)CONTAINER::get_next_memory_block(hook->gameMemory);
	//CONTAINER::increase_memory_block(&hook->gameMemory,sizeof(int) * MAX_RENDER_OBJECTS);


	Material planetMat = create_new_material(hook->shaders,"MainProg");
	Material lattiaMat = create_new_material(hook->shaders,"AnimatedProg");
	Material ManMaterial = create_new_material(hook->shaders,"AnimatedProg");

	RenderData lattia = create_new_renderdata
		(
		 MaterialType::Lattia,
		 get_model(hook->models,"Skeleton"),
		 MATH::vec3(0.f,3.0f,0.f),
		 MATH::quaternion(MATH::vec3(MATH::deg_to_rad * -90.f,0.f,0.f)),
		 MATH::vec3(0.001f,0.001f,0.001f)
		);
	RenderData cube = create_new_renderdata
		(
		 MaterialType::PlanetMat,
		 get_model(hook->models,"Cube"),
		 MATH::vec3(0.f,0.f,0.f),
		 MATH::quaternion(),
		 MATH::vec3(10.f,1.0f,10.f)
		);
#if 1
	RenderData man = create_new_renderdata
		(
		 MaterialType::Man,
		 get_model(hook->models,"Man"),
		 MATH::vec3(5.f,1.f,3.f),
		 MATH::quaternion(MATH::vec3(MATH::deg_to_rad * -90.f,
				 MATH::deg_to_rad * -90.f,0.f)),
		 MATH::vec3(0.6f,0.6f,0.6f)
		);
#endif
	AnimeHandle handle = create_new_animation(0,get_model(hook->models,"Skeleton"),1.f);
	AnimeHandle manAnim = create_new_animation(0,get_model(hook->models,"Man"),1.f);
	lattia.animationIndex = handle;
	man.animationIndex = manAnim;
	insert_renderdata(lattia,&game->renderData,hook->renderables);
	insert_renderdata(cube,&game->renderData,hook->renderables);
	cube.scale = MATH::vec3(1.f,1.f,1.f);
	cube.position = MATH::vec3(3.f,3.f,1.f);
	insert_renderdata(cube,&game->renderData,hook->renderables);
	insert_renderdata(man,&game->renderData,hook->renderables);

	hook->numRenderables = 4;
	set_material_texture(hook->shaders,&planetMat,0,get_texture(*hook->textures,"Lattia"));
	set_material_texture(hook->shaders,&lattiaMat,0,get_texture(*hook->textures,"Lattia"));
	set_material_texture(hook->shaders,&ManMaterial,0,get_texture(*hook->textures,"Skin"));
	hook->materials[0] = planetMat;
	hook->materials[1] = lattiaMat;
	hook->materials[2] = ManMaterial;
	init_sound_device(&game->soundContext,&hook->workingMemory,&hook->gameMemory);

	init_camera(&game->camera,&hook->viewMatrix,&hook->projectionMatrix,
			MATH::vec3(0,5,-10.f),MATH::vec3(4.f,0,-4.f));


	//ImGui::SetCurrentContext(hook->imguiContext);
	set_input_context(&hook->inputs);
#if MIKA
	game->client.init_client("172.31.16.152",60000,"Nilkki");
	game->client.OpenConnection();
	printf("connected \n");
#endif
#if 0
	btCollisionConfiguration* collisionConfig = new btDefaultCollisionConfiguration();
	btDispatcher* dispatcher = new btCollisonDispatcher(collisionConfig);
	btBroadphaseInterface* broadPhase = new btDbvtBroadphase();
	btDynamicsWorld* world;
	btConstraintRow* solver;
#endif
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration(); 
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase ();
	btSequentialImpulseConstraintSolver* solver = new  btSequentialImpulseConstraintSolver;

	game->dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher ,
			overlappingPairCache ,solver ,collisionConfiguration);

	game->dynamicsWorld ->setGravity(btVector3 (0,-4.f,0));

	{
		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0,-0.5,0));
		btBoxShape* groundShape = new btBoxShape(btVector3(btScalar(10.f),btScalar(1.),btScalar(10.)));

		//game->collisionShapes[game->numShapes++] = groundShape; 
		btScalar mass(0.);
		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass,localInertia);

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,groundShape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		//add the body to the dynamics world
		game->dynamicsWorld->addRigidBody(body);
	}
	{
		btBoxShape* colShape = new btBoxShape(btVector3(1.f,1.f,1.f));
		//btCollisionShape* colShape = new btSphereShape(btScalar(1.));
		//game->collisionShapes[game->numShapes++] = colShape; 
		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();
		btScalar	mass(1.f);
		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass,localInertia);

		for(int box = 0; box < 5;box++)
		{
			startTransform.setOrigin(btVector3((float)(-box) * 1.1f,4.f,-3.f));
			btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
			btRigidBody* body = new btRigidBody(rbInfo);
			game->dynamicsWorld->addRigidBody(body);

			PhysicsBox* currentBox = &game->boxes[box];
			currentBox->body = body;

			RenderData boxRenderData = create_new_renderdata
				(
				 MaterialType::PlanetMat,
				 get_model(hook->models,"Cube"),
				 MATH::vec3( (float)(-box * 3.f),4.f,-5.f),
				 MATH::quaternion(),
				 MATH::vec3(1.0f,1.0f,1.0f)
				);

			currentBox->boxRenderData = insert_renderdata(boxRenderData,&game->renderData,hook->renderables);
			hook->numRenderables++;
			game->numBoxes++;
		}
	}
}

EXPORT void on_game_reload(void* p)
{
	GameHook* hook = (GameHook*)p;
	Game* game = (Game*)hook->userData;
	set_sound_context(&game->soundContext);
	//ImGui::SetCurrentContext(hook->imguiContext);
	set_input_context(&hook->inputs);
	set_animation_context(&game->animations);
}

static void update_camera(Camera* cam,MATH::mat4* view)
{
	MATH::vec2 movement = get_mouse_movement();
#if 0
	if (key_down(Key::KEY_M))
		movement.x += 6.1f;
	if (key_down(Key::KEY_N))
		movement.y += 6.1f;
	if (key_down(Key::KEY_V))
		movement.x -= 6.1f;
	if (key_down(Key::KEY_B))
		movement.y -= 6.1f;
#endif


	float sensitivity = 0.05f;
	scale(&movement,sensitivity);

	//printf("%.3f %.3f \n",cam->yaw, cam->pitch);
	cam->yaw   += movement.x;
	cam->pitch -= movement.y;  

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

	MATH::create_lookat_mat4(view,cam->position,
			cam->position + cam->direction, cam->up);

	MATH::mat4 inv;
	MATH::inverse_mat4(&inv,view);
}



EXPORT void update_game(void* p)
{
	GameHook* hook = (GameHook*)p;
	Game* game = (Game*)hook->userData;
	//ImGui::Begin("Game window ");
	//ImGui::Text("lenght of song is %.3f \n",0.f);
	static bool pause = true;
	//ImGui::Checkbox("pause",&pause);
	//ImGui::End();

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
	game->dynamicsWorld ->stepSimulation (1.f/60.f,10);
	for (int j = game->dynamicsWorld->getNumCollisionObjects () -1; j>=0 ;j--)
	{
		btCollisionObject* obj = game->dynamicsWorld->getCollisionObjectArray ()[j];
		btRigidBody* body = btRigidBody :: upcast(obj);
		if (body && body->getMotionState())
		{
			btTransform  trans;
			body->getMotionState ()->getWorldTransform(trans);
			printf("world pos = %f,%f,%f\n",float(trans.getOrigin ().getX()),float(trans.
						getOrigin ().getY()),float(trans.getOrigin ().getZ()));
		}
	}
	for(uint i = 0; i < game->numBoxes;i++)
	{
		btRigidBody* body = game->boxes[i].body;
		btTransform trans;
		body->getMotionState()->getWorldTransform(trans);
		btVector3 origin = trans.getOrigin();

		RenderData* data = get_render_data(game->boxes[i].boxRenderData,game->renderData,hook->renderables);
		data->position.x = origin.getX();
		data->position.y = origin.getY();
		data->position.z = origin.getZ();

		btQuaternion q = trans.getRotation();
		//MATH::vec3 euler();
		data->orientation.scalar = q.getW();
		data->orientation.i = q.getX();
		data->orientation.j = q.getY();
		data->orientation.k = q.getZ();
	}

	if(key_pressed(Key::KEY_M))
	{
		btBoxShape* colShape = new btBoxShape(btVector3(1.f,1.f,1.f));
		//btCollisionShape* colShape = new btSphereShape(btScalar(1.));
		//game->collisionShapes[game->numShapes++] = colShape; 
		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();
		btScalar	mass(1.f);
		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,0,0);
		if (isDynamic){
			colShape->calculateLocalInertia(mass,localInertia);
		}

		startTransform.setOrigin(btVector3(
					game->camera.position.x,game->camera.position.y,game->camera.position.z));
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		game->dynamicsWorld->addRigidBody(body);
		body->applyCentralForce(btVector3(game->camera.direction.x,
					game->camera.direction.y,
					game->camera.direction.z) * 1000);

		PhysicsBox* currentBox = &game->boxes[game->numBoxes];
		currentBox->body = body;

		RenderData boxRenderData = create_new_renderdata
			(
			 MaterialType::PlanetMat,
			 get_model(hook->models,"Cube"),
			 MATH::vec3( game->camera.position),
			 MATH::quaternion(),
			 MATH::vec3(1.0f,1.0f,1.0f)
			);

		currentBox->boxRenderData = insert_renderdata(boxRenderData,&game->renderData,hook->renderables);
		hook->numRenderables++;
		game->numBoxes++;
	}

	update_camera(&game->camera,&hook->viewMatrix);
#if MIKA
	game->client.Update();
#endif
}

EXPORT void dispose_game(void* p)
{
	printf("disposing game \n");
	GameHook* hook = (GameHook*)p;
	Game* game = (Game*)hook->userData;
	alcMakeContextCurrent(NULL);
	alcDestroyContext(game->soundContext.context);
	alcCloseDevice(game->soundContext.device);
	printf("game disposed\n");
}



