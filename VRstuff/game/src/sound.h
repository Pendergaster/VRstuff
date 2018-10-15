#ifndef PAKKI_SOUNDS
#define PAKKI_SOUNDS
#include <Utils.h>
#include <string.h>
#include <Containers.h>
#include <MathUtil.h>
#define OpAL 1
#if OpAL
#include <AL/al.h>
#include <AL/alc.h>
//#include <alut.h>
#endif
#if OpAL

#define GENERATE_SOUND_STRING(STRING) "assets/"#STRING".wav",

#define SOUNDS(MODE)\
	MODE(sound)\


enum SoundType : int
{
	SOUNDS(GENERATE_ENUM)
		MaxSounds
};

const char* sound_names[] = 
{
	SOUNDS(GENERATE_STRING)
};


static void _list_audio_devices(const ALCchar *devices)
{
	const ALCchar *device = devices, *next = devices + 1;
	size_t len = 0;

	fprintf(stdout, "Devices list:\n");
	fprintf(stdout, "----------\n");
	while (device && *device != '\0' && next && *next != '\0') {
		fprintf(stdout, "%s\n", device);
		len = strlen(device);
		device += (len + 1);
		next += (len + 2);
	}
	fprintf(stdout, "----------\n");
}

#define list_audio_devices() _list_audio_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER))
void _OpenALErrorCheck(const char* file,unsigned int line)
{
	ALCenum error;

	error = alGetError();
	if (error != AL_NO_ERROR)
	{
		ABORT_MESSAGE("OPENAL ERROR DETECTED IN :: %s | line %d\n",file,line);
	}
}
#define OpenALErrorCheck() _OpenALErrorCheck(__FILE__, __LINE__)
#endif


struct SoundLoadInfo
{
	unsigned long	size;
	unsigned long	chunkSize;
	short			formatType;
	short			channels;
	unsigned long	sampleRate;
	unsigned long	avgBytesPerSec;
	short			bytesPerSample;
	short			bitsPerSample;
	unsigned long	dataSize;
};

struct SoundInfo
{
	//ALuint source = 0;
	ALuint buffer = 0;
	ALuint frequency = 0;//sampleRate;
	ALuint format = 0;
};

struct SoundPlayer
{
	ALuint		source = 0;
	float		pitch = 0;
	float		gain = 0;
};
#define MAX_PLAYERS 100
struct SoundContext
{
	ALCdevice*		device = NULL;
	ALCcontext*		context = NULL;
	SoundInfo*		soundInfos = NULL;
	MATH::vec3		listenerPos;
	MATH::vec3		orientationAT;
	MATH::vec3		orientationUP;
	MATH::vec3		listenerVelocity;
	int				playerIndex = 0;
	SoundPlayer*	players = NULL;
	int				playerFreeListIndex = 0;
	uint*			playerFreeList = NULL;
};

static SoundContext* g_soundContext = NULL;

SoundLoadInfo load_wav(const char* path,unsigned char** buf,CONTAINER::MemoryBlock* workMem)
{
	FILE *fp = fopen(path,"rb");
	defer{fclose(fp);};
	ASSERT_MESSAGE(fp,"sound not found %s \n",path);
	char type[4];
	SoundLoadInfo info;

	fread(type,sizeof(char),4,fp);
	if(type[0] != 'R' || type[1] != 'I' || type[2] != 'F' || type[3] != 'F')
	{
		ABORT_MESSAGE("sound is not RIFF \n");
	}

	fread(&info.size,sizeof(info.size),1,fp);
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
	fread(&info.chunkSize,sizeof(info.chunkSize),1,fp);
	fread(&info.formatType,sizeof(short),1,fp);
	fread(&info.channels,sizeof(short),1,fp);
	fread(&info.sampleRate,sizeof(info.sampleRate),1,fp);
	fread(&info.avgBytesPerSec,sizeof(info.avgBytesPerSec),1,fp);
	fread(&info.bytesPerSample,sizeof(short),1,fp);
	fread(&info.bitsPerSample,sizeof(short),1,fp);

	fread(type,sizeof(char),4,fp);
	if(type[0] != 'd' || type[1] != 'a' || type[2] != 't' || type[3] != 'a')
	{
		ASSERT_MESSAGE(false,"missing sound data! ,%c ,%c ,%c ,%c \n",type[0],type[1],type[2],type[3]);
	}

	fread(&info.dataSize,sizeof(info.dataSize),1,fp);
	*buf = (unsigned char*)CONTAINER::get_next_memory_block(*workMem);
	CONTAINER::increase_memory_block_aligned(workMem,sizeof(unsigned char) * info.dataSize);
	fread(buf,sizeof(unsigned char),info.dataSize,fp);

	return info;
}

static void init_sound_device(SoundContext* soundCon,CONTAINER::MemoryBlock* workMem,CONTAINER::MemoryBlock* staticMem)
{
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
	soundCon->soundInfos = (SoundInfo*)CONTAINER::get_next_memory_block(*staticMem);
	CONTAINER::increase_memory_block(staticMem,sizeof(SoundInfo) * SoundType::MaxSounds);

	soundCon->players = (SoundPlayer*)CONTAINER::get_next_memory_block(*staticMem);
	CONTAINER::increase_memory_block(staticMem,sizeof(SoundPlayer) * MAX_PLAYERS);

	soundCon->playerFreeList = (uint*)CONTAINER::get_next_memory_block(*staticMem);
	CONTAINER::increase_memory_block(staticMem,sizeof(uint) * MAX_PLAYERS);
	SoundInfo* currentInfo = soundCon->soundInfos;
	for(char* name = (char*)sound_names;name;name++ ,currentInfo++)
	{
		unsigned char* buf = NULL;
		CONTAINER::MemoryBlock prevState = *workMem;
		SoundLoadInfo loadInfo = load_wav(name,&buf,workMem);
		currentInfo->frequency = loadInfo.sampleRate;
		if(8 == loadInfo.bitsPerSample )
		{
			if(1 == loadInfo.channels ){
				currentInfo->format = AL_FORMAT_MONO8;
			}
			else if(2 == loadInfo.channels ){
				currentInfo->format = AL_FORMAT_STEREO8;
			}
		}
		else if(16 == loadInfo.bitsPerSample)
		{
			if(1 == loadInfo.channels ){
				currentInfo->format = AL_FORMAT_MONO16;
			}
			else if(2 == loadInfo.channels ){
				currentInfo->format = AL_FORMAT_STEREO16;
			}
		}else {ABORT_MESSAGE("SOMETHING FAILED \n");}
		alGenBuffers(1,&currentInfo->buffer);
		//alGenSources(1,&currentInfo->source);
		alBufferData(currentInfo->buffer,currentInfo->format,buf,
				loadInfo.dataSize,currentInfo->frequency);

		//ALfloat sourceVel[] = { 0.0f, 0.0f, 0.0f };
		//ALfloat sourcePos[] = { 0.0f, 0.0f, 0.0f };
		*workMem = prevState;
		OpenALErrorCheck();
	}
	soundCon->listenerPos = MATH::vec3(0.0f, 0.0f, 0.0f );
	soundCon->orientationAT = MATH::vec3(0.0f, 0.0f, -1.0f );
	soundCon->orientationUP = MATH::vec3(0.0f, 1.0f, 0.0f );
	alListenerfv(AL_POSITION,(float*)&soundCon->listenerPos);
	alListenerfv(AL_VELOCITY,(float*)&soundCon->listenerVelocity);
	alListenerfv(AL_ORIENTATION,(float*)&soundCon->orientationAT);

	g_soundContext = soundCon;
}

typedef uint PlayerHandle;

static inline PlayerHandle get_new_player()
{
	ASSERT_MESSAGE(g_soundContext,"SOUND CONTEXT NOT SET \n");
	uint ret = 0;
	SoundPlayer* player = NULL;
	if(0 < g_soundContext->playerFreeListIndex)
	{
		--g_soundContext->playerFreeListIndex;
		ret = g_soundContext->playerFreeList[g_soundContext->playerFreeListIndex];
	}
	else
	{
		ASSERT_MESSAGE(g_soundContext->playerIndex < MAX_PLAYERS,"TOO MANY SOUND PLAYERS \n");
		ret = g_soundContext->playerIndex;
		g_soundContext->playerIndex++;
	}
	player = &g_soundContext->players[ret];

	alGenSources(1,&player->source);

	OpenALErrorCheck();
	return 0;
}

static inline void set_player(PlayerHandle handle,SoundType sound,MATH::vec3 pos, MATH::vec3 vel,float gain,float pitch,bool loopping)
{
	SoundPlayer* player = &g_soundContext->players[handle];
	SoundInfo* info = &g_soundContext->soundInfos[sound];

	alSourcei(player->source,AL_BUFFER,info->buffer);
	alSourcef(player->source,AL_PITCH,pitch);
	alSourcef(player->source,AL_GAIN,gain);
	alSourcefv(player->source,AL_POSITION,(float*)&pos);
	alSourcefv(player->source,AL_VELOCITY,(float*)&vel);
	alSourcei(player->source,AL_LOOPING,loopping ?  AL_TRUE : AL_FALSE);
	OpenALErrorCheck();

	player->pitch = pitch;
	player->gain = gain;
}
static inline void set_listener_worldData(PlayerHandle handle,
		const MATH::vec3& pos,const MATH::vec3& vel,const MATH::vec3& at,
		const MATH::vec3& up)
{
	SoundPlayer* player = &g_soundContext->players[handle];
	alSourcefv(player->source,AL_POSITION,(float*)&pos);
	OpenALErrorCheck();


	g_soundContext->listenerPos = pos;
	g_soundContext->listenerVelocity = vel;
	g_soundContext->orientationAT = at;
	g_soundContext->orientationUP = up;
	alListenerfv(AL_POSITION,(float*)&g_soundContext->listenerPos);
	alListenerfv(AL_VELOCITY,(float*)&g_soundContext->listenerVelocity);
	alListenerfv(AL_ORIENTATION,(float*)&g_soundContext->orientationAT);
}

static inline void set_player_pos(PlayerHandle handle,const MATH::vec3& pos)
{
	SoundPlayer* player = &g_soundContext->players[handle];
	alSourcefv(player->source,AL_POSITION,(float*)&pos);
	OpenALErrorCheck();
}
static inline void set_player_vel(PlayerHandle handle,const MATH::vec3& vel)
{
	SoundPlayer* player = &g_soundContext->players[handle];
	alSourcefv(player->source,AL_VELOCITY,(float*)&vel);
	OpenALErrorCheck();
}
static inline void set_player_pitch(PlayerHandle handle,float pitch)
{
	SoundPlayer* player = &g_soundContext->players[handle];
	alSourcef(player->source,AL_PITCH,pitch);
	OpenALErrorCheck();
	player->pitch = pitch;
}
static inline void set_player_gain(PlayerHandle handle,float gain)
{
	SoundPlayer* player = &g_soundContext->players[handle];
	alSourcef(player->source,AL_PITCH,gain);
	OpenALErrorCheck();
	player->gain = gain;
}
static inline void player_play(PlayerHandle handle)
{
	SoundPlayer* player = &g_soundContext->players[handle];
	alSourcePlay(player->source);
	OpenALErrorCheck();
}
static inline void player_pause(PlayerHandle handle)
{
	SoundPlayer* player = &g_soundContext->players[handle];
	alSourcePause(player->source);
	OpenALErrorCheck();
}







#endif
