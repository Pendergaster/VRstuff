#ifndef PAKKI_TEXTUREDEFS
#define PAKKI_TEXTUREDEFS
#include <Utils.h>
#include <Containers.h>

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

#define TEXTURE_TYPES(MODE)\
	MODE(Texture2D)\
	MODE(CubeMap)\


const char* TEXTURE_TYPE_NAMES[] = {
	TEXTURE_TYPES(GENERATE_STRING)
};

enum TextureType : int 
{
	TEXTURE_TYPES(GENERATE_ENUM)
		MaxTexTypes
};


typedef int TextureID;

struct TextureInfo
{
	char*		name = NULL;
	union{
		char* 		path;
		struct{
			char* topPath,*downPath,*frontPath,*backPath,*rightPath,*leftPath;
		};
	};
	bool 		mipmap = false;
	bool 		srgb = false;
	int			widht = 0;
	int			height = 0;
	int			channels = 0;
	int 		wrapMode = WrapMode::Repeat; 
	int			texType = TextureType::Texture2D;
};
struct TextureData
{
	uint						numTextures = 0;
	CONTAINER::StringTable<int> textureCache;
	uint*						textureIds = NULL;
	TextureInfo*				textureInfos = NULL;
};

static TextureID get_texture(const TextureData& textures,const char* name)
{
	int* ret = CONTAINER::access_table<int>(textures.textureCache,name);
	ASSERT_MESSAGE(ret,"TEXTURE NOT FOUND :: %s \n",name);
	return *ret;
}

#endif// PAKKI_TEXTUREDEFS
