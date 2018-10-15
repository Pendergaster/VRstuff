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



struct Game
{
	int*	renderablesFreeList = NULL;	
	int		renderablesFreelistSize = 0;
};
#define MAX_RENDER_OBJECTS 100
RenderData create_new_renderdata(uint materialID, uint meshID,const MATH::vec3& pos,float scale)
{
	RenderData ret;
	ret.materialID = materialID;
	ret.meshID = meshID;
	ret.position = pos;
	ret.scale = scale;
	return ret;
}

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

	RenderData planetData;
	planetData.materialID = MaterialType::PlanetMat;
	planetData.meshID = get_mesh(hook->meshes,"Planet");
	planetData.orientation = MATH::quaternion();
	planetData.position = MATH::vec3(0,0,0);
	planetData.scale = 0.5f;
	planetData.oriTemp = MATH::vec3();

	TextureID moonTex = get_texture(*hook->textures,"MoonTexture");
	set_material_texture(hook->shaders,&planetMat,0,moonTex);
	hook->materials[0] = planetMat;
	hook->renderables[0] = planetData;
	hook->renderIndexes[0] = 0;
	hook->numRenderables = 1;

	SoundContext sounds;
	//todo init working mem ymsyms
	init_sound_device(&sounds,&hook->workingMemory,&hook->gameMemory);

#if 0//OpAL
	ALCdevice *device;

	device = alcOpenDevice(NULL);
	if (!device)
	{
		ABORT_MESSAGE("FAILED TO GET AUDIO DEVICE \n");
	}

	list_audio_devices();

	ALCcontext *context;
	context = alcCreateContext(device, NULL);
	if (!alcMakeContextCurrent(context))
	{
		ABORT_MESSAGE("FAILED TO GET AUDIO DEVICE \n");
	}

	OpenALErrorCheck();
	//generate source
#if 0
	ALuint source;
	alGenSources((ALuint)1, &source);
	OpenALErrorCheck();

	alSourcef(source, AL_PITCH, 1);
	OpenALErrorCheck();
	alSourcef(source, AL_GAIN, 1);
	OpenALErrorCheck();
	alSource3f(source, AL_POSITION, 0, 0, 0);
	OpenALErrorCheck();
	alSource3f(source, AL_VELOCITY, 0, 0, 0);
	OpenALErrorCheck();
	alSourcei(source, AL_LOOPING, AL_FALSE);
	OpenALErrorCheck();

	//generate buffers
	ALuint buffer;

	alGenBuffers((ALuint)1, &buffer);
	OpenALErrorCheck();
#endif
	//parse WAV
	FILE *fp = fopen("sound.wav","rb");
	ASSERT_MESSAGE(fp,"sound.wav not found \n");
	char type[4];
	DWORD size,chunkSize;
	short formatType,channels;
	DWORD sampleRate,avgBytesPerSec;
	short bytesPerSample, bitsPerSample;
	DWORD dataSize;

	fread(type,sizeof(char),4,fp);
	if(type[0] != 'R' || type[1] != 'I' || type[2] != 'F' || type[3] != 'F')
	{
		ABORT_MESSAGE("sound is not RIFF \n");
	}

	fread(&size,sizeof(DWORD),1,fp);
	fread(type,sizeof(char),4,fp);
	if(type[0] != 'W' || type[1] != 'A' || type[2] != 'V' || type[3] != 'E')
	{
		ABORT_MESSAGE("sound is not WAVE \n");
	}

	fread(type,sizeof(char),4,fp);
	if(type[0] != 'f' || type[1] != 'm' || type[2] != 't' || type[3] != ' ')
	{
		ABORT_MESSAGE("sound is not FMT \n");
	}

	fread(&chunkSize,sizeof(DWORD),1,fp);
	fread(&formatType,sizeof(short),1,fp);
	fread(&channels,sizeof(short),1,fp);
	fread(&sampleRate,sizeof(DWORD),1,fp);
	fread(&avgBytesPerSec,sizeof(DWORD),1,fp);
	fread(&bytesPerSample,sizeof(short),1,fp);
	fread(&bitsPerSample,sizeof(short),1,fp);

	fread(type,sizeof(char),4,fp);
	if(type[0] != 'd' || type[1] != 'a' || type[2] != 't' || type[3] != 'a')
	{
		ASSERT_MESSAGE(false,"missing sound data! ,%c ,%c ,%c ,%c \n",type[0],type[1],type[2],type[3]);
	}
	fread(&dataSize,sizeof(DWORD),1,fp);
	unsigned char* buf = (unsigned char*)malloc(sizeof(unsigned char) * dataSize);
	fread(buf,sizeof(BYTE),dataSize,fp);

	ALuint source,buffer,frequency = sampleRate,format = 0;
	alGenBuffers(1,&buffer);
	alGenSources(1,&source);

	if(8 ==bitsPerSample )
	{
		if(1 == channels ){
			format = AL_FORMAT_MONO8;
		}
		else if(2 == channels ){
			format = AL_FORMAT_STEREO8;
		}
	}
	else if(16 == bitsPerSample)
	{
		if(1 == channels ){
			format = AL_FORMAT_MONO16;
		}
		else if(2 == channels ){
			format = AL_FORMAT_STEREO16;
		}
	}else {ABORT_MESSAGE("SOMETHING FAILED \n");}
	alBufferData(buffer,format,buf,dataSize,frequency);

	ALfloat sourceVel[] = { 0.0f, 0.0f, 0.0f };
	ALfloat sourcePos[] = { 0.0f, 0.0f, 0.0f };
	ALfloat listenerPos[] = { 0.0f, 0.0f, 0.0f };
	ALfloat listenerVel[] = { 0.0f, 0.0f, 0.0f };
	ALfloat listenerOri[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	//listener
	alListenerfv(AL_POSITION,listenerPos);
	alListenerfv(AL_VELOCITY,listenerVel);
	alListenerfv(AL_ORIENTATION,listenerOri);
	//source
	alSourcei(source,AL_BUFFER,buffer);
	alSourcef(source,AL_PITCH,0.5f);
	alSourcef(source,AL_GAIN,0.8f);
	alSourcefv(source,AL_POSITION,sourcePos);
	alSourcefv(source,AL_LOOPING,sourceVel);
	alSourcei(source,AL_LOOPING,AL_TRUE);
#if 0
	alListener3f(AL_POSITION, 0, 0, 1.0f);
	OpenALErrorCheck();
	alListener3f(AL_VELOCITY, 0, 0, 0);
	OpenALErrorCheck();
	alListenerfv(AL_ORIENTATION, listenerOri);
	OpenALErrorCheck();




	alSourcef(source, AL_PITCH, 1);
	OpenALErrorCheck();
	alSourcef(source, AL_GAIN, 1);
	OpenALErrorCheck();
	alSource3f(source, AL_POSITION, 0, 0, 0);
	OpenALErrorCheck();
	alSource3f(source, AL_VELOCITY, 0, 0, 0);
	OpenALErrorCheck();
	alSourcei(source, AL_LOOPING, AL_FALSE);
	OpenALErrorCheck();

#endif
	alSourcePlay(source);
	OpenALErrorCheck();
	printf("SOUNDS INITIALIZED \n");
	fclose(fp);
	//defer{free(buf);};
	//defer{alDeleteSources(1,&source);};
	//defer{alDeleteBuffers(1,&buffer);};
	//defer{alcMakeContextCurrent(NULL);};
	//defer{alcDestroyContext(context);};
	//defer{alcCloseDevice(device);};
#endif



	printf("Game INITED \n");
}

EXPORT void update_game(void* p)
{
	static bool load = true;
	GameHook* hook = (GameHook*)p;
	if(load)
	{
		//ImGuiIO& io = *hook->imguiContext;(void)io;
		//ImGui::GetCurrentContext();
		ImGui::SetCurrentContext(hook->imguiContext);
		load = false;
		printf("im loaded again\n");
	}
	MATH::quaternion rot(MATH::vec3(0,1,0),MATH::deg_to_rad * 50.f);
	static float dt = 0;
	dt += 1.f / 60.f;
	hook->renderables->position.x = -sin(dt/2.f); 	
	hook->renderables->position.y = -sin(dt); 	
	hook->renderables->position.z = -sin(dt/2.f); 	
	hook->renderables->oriTemp.y  += -2.f;
	hook->renderables->oriTemp.x  += 2.f;

	//ImGui::Begin("Game window ");
	//	ImGui::Text("oot homo petteri ");
	//	ImGui::End();
	//GameHook* hook = (GameHook*)p;
	//printf(" update \n");
}
