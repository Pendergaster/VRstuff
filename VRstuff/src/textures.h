#ifndef PAKKI_TEXTURES
#define PAKKI_TEXTURES
typedef int TextureID;
#include <Containers.h>
#include <JsonToken.h>
#include <Utils.h>
#include<stb_image.h>
#include <glad/glad.h>
#include "glerrorcheck.h"
#define WRAP_MODES(MODE)\
	MODE(Repeat)\
MODE(MirroredRepeat)\
MODE(ClampToEdge)\
MODE(ClampToBorder)\


const char* WRAP_MODE_NAMES[] = {
	WRAP_MODES(GENERATE_STRING)
};

enum WrapMode : int 
{
	WRAP_MODES(GENERATE_ENUM)
		MaxModes
};

struct TextureInfo
{
	char*		name = NULL;
	char* 		path = NULL;
	bool 		mipmap = false;
	bool 		srgb = false;
	int			widht = 0;
	int			height = 0;
	int			channels = 0;
	int 		wrapMode = WrapMode::Repeat; 
};
struct TextureData
{
	uint						numTextures = 0;
	CONTAINER::StringTable<int> textureCache;
	uint*						textureIds = NULL;
	TextureInfo*				textureInfos = NULL;
};

static inline bool load_texture(TextureInfo* info,uint id)
{
	//int widht = 0 ,height = 0 ,channels = 0;
	TextureInfo temp = *info;
	unsigned char* data = stbi_load(temp.path, &temp.widht, &temp.height, &temp.channels,0);
	//ASSERT_MESSAGE(data,"FAILED TO LOAD TEXTURE :: %s \n",temp.name);
	if (!data) return false;
	defer {stbi_image_free(data);};

	glBindTexture(GL_TEXTURE_2D,id);
	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,temp.wrapMode);
	GLenum mode = 0;
	if(temp.wrapMode == WrapMode::ClampToBorder){
		mode = GL_CLAMP_TO_BORDER;
	}
	else if(temp.wrapMode == WrapMode::ClampToEdge){
		mode = GL_CLAMP_TO_EDGE;
	}
	else if(temp.wrapMode == WrapMode::MirroredRepeat){
		mode = GL_MIRRORED_REPEAT;
	}
	else if(temp.wrapMode == WrapMode::Repeat){
		mode = GL_REPEAT;
	}

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,mode);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,mode);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLenum mode3 = temp.channels == 3 ? GL_RGB : GL_RGBA;
	GLenum mode2 = temp.srgb ? (temp.channels == 3 ? GL_SRGB : GL_SRGB_ALPHA) : mode3;

	glTexImage2D(GL_TEXTURE_2D,0,mode2,temp.widht,temp.height,0,mode3,GL_UNSIGNED_BYTE,data);

	if(temp.mipmap){
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	*info = temp;
	return true;
}
static int get_texture(const TextureData& textures,const char* name)
{
	int* ret = CONTAINER::access_table<int>(textures.textureCache,name);
	ASSERT_MESSAGE(ret,"TEXTURE NOT FOUND :: %s \n",name);
	return *ret;
}
//TODO define stb malloc ja realloc ja free
//TODO testaa json true ja false
//TODO siivoo kommentit
static void init_textures_from_metadata(TextureData* textureData,
		CONTAINER::MemoryBlock* staticMem)
	//JsonToken* token,uint* textureIds,TextureInfo* textureInfos,int* numTextures,
	//CONTAINER::StringTable<int>* textureTable,CONTAINER::DynamicArray<char*> names,
	//CONTAINER::MemoryBlock* staticMem)
{
	JsonToken token;
	token.ParseFile("importdata/texturedata.json");
	CONTAINER::DynamicArray<char*> names;
	CONTAINER::init_dynamic_array(&names);
	defer{CONTAINER::dispose_dynamic_array(&names);};
	token.GetKeys(&names);

	textureData->textureInfos = (TextureInfo*)CONTAINER::get_next_memory_block(*staticMem);
	CONTAINER::increase_memory_block(staticMem,names.numobj * sizeof(TextureInfo));
	textureData->textureIds = (uint*)CONTAINER::get_next_memory_block(*staticMem);
	CONTAINER::increase_memory_block(staticMem,names.numobj * sizeof(uint));

	glGenTextures(names.numobj,textureData->textureIds);
	textureData->numTextures = names.numobj;
	CONTAINER::init_table_with_block<int>(&textureData->textureCache,staticMem,
			(int)textureData->numTextures);
	for(uint i = 0; i < names.numobj ; i++)
	{
		char* currentName = names.buffer[i];
		JsonToken* currentToken = token[currentName].GetToken();
		ASSERT_MESSAGE(currentToken,"TEXTURE DATA IS NOT VALID :: %s \n",currentName);
		char* path = (*currentToken)["Path"].GetString();
		ASSERT_MESSAGE(path,"TEXTURE PATH IS NOT VALID :: %s \n",currentName);
		TextureInfo info;
        info.path = (char*)CONTAINER::get_next_memory_block(*staticMem);
		strcpy(info.path,path);
		CONTAINER::increase_memory_block_aligned(staticMem,(int)strlen(info.path)+1);
		//info.channels = channels;
		//info.height = height;
		//info.widht = widht;
		info.name = (char*)CONTAINER::get_next_memory_block(*staticMem);
		strcpy(info.name,currentName);
		CONTAINER::increase_memory_block_aligned(staticMem,(int)strlen(info.name)+1);
		JValueHandle val = (*currentToken)["Mipmap"];
		ASSERT_MESSAGE(val.IsValid(),"MIPMAP IS NOT DEFINED :: %s \n",currentName);
		info.mipmap = val.GetBool();
		val = (*currentToken)["SRGB"];
		ASSERT_MESSAGE(val.IsValid(),"MIPMAP IS NOT DEFINED :: %s \n",currentName);
		info.srgb = val.GetBool();

		char* wrapMode = (*currentToken)["Wrap"].GetString();
		ASSERT_MESSAGE(wrapMode,"WRAP MODE NOT DEFINED FOR %s",info.name);
		int i2 = 0;
		for(; i2 < WrapMode::MaxModes; i2++)
		{
			if(!strcmp(wrapMode,WRAP_MODE_NAMES[i2]))
			{
				info.wrapMode = i2;	
				break;
			}
		}
		ASSERT_MESSAGE(i2 < WrapMode::MaxModes,"WRAPMODE NOT DEFINED FOR %s",info.name);
		//TODO  lataa opngl puoli
		CONTAINER::insert_to_table<int>(&textureData->textureCache,info.name,i);
		if(!load_texture(&info,textureData->textureIds[i]))
		{
			ABORT_MESSAGE("FAILED TO LOAD TEXTURE %s \n",currentName);
		}
		textureData->textureInfos[i] = info;
	}
}




#endif
