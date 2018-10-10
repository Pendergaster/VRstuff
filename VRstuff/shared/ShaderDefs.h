#ifndef PAKKI_SHADER_DEFS
#define PAKKI_SHADER_DEFS
#include <Containers.h>
#include <file_system.h>
#include <Utils.h>
#include <MathUtil.h>
//#include <shader_utils.h>
#define UNIFORMTYPES(MODE)\
	MODE(INVALID)\
	MODE(INTTYPE)\
	MODE(FLOATTYPE)\
	MODE(VEC4)\
	MODE(MAT4)\
	MODE(SAMPLER2D)\
	MODE(SAMPLERCUBE)\
	MODE(MODEL)\

#define RENDERGROUPS(MODE)\
	MODE(INVALID)\
	MODE(Model)\
	MODE(PostProcess)\


enum UniformType : int
{
	UNIFORMTYPES(GENERATE_ENUM)
		MaxTypes
};

const char* UNIFORM_TYPE_NAMES[] = 
{
	UNIFORMTYPES(GENERATE_STRING)
};

struct UniformInfo
{
	int 	type = UniformType::INVALID;
	union{
		int 	location;
		int		glTexLocation;
	};
	char* 	name = NULL;
	int 	hashedID = 0;	
};

enum GlobalUniforms
{
	Invalid = 0x0,
	MVP = 0x1,
	GlobalLight = 0x2,
};

struct Uniform
{
	int 	type = UniformType::INVALID;
	union 
	{
		int 				_int;	
		uint				_textureCacheId;	
		float 				_float;
		MATH::vec4			_vec4;
		MATH::mat4 			_mat4;
	};
};
enum class ShaderType : int
{
	Normal, // vert and frag in different files
	Combined, // vert and frag in same file
};
enum class RenderGroup : int
{
	RENDERGROUPS(GENERATE_ENUM)
		MaxTypes
};

const char* RENDERGORUP_NAMES[] = 
{
	RENDERGROUPS(GENERATE_STRING)
};

struct ShaderProgram
{
	// used for setting up material
	uint 			numTextures = 0;
	// set before rendering
	uint 			numUniforms = 0;
	//uniforms info for setting material for rendering
	UniformInfo* 	uniforms;
	ShaderType      type = ShaderType::Normal;
	RenderGroup     group = RenderGroup::INVALID;
	int				globalUniformFlags = 0;
	//TODO set this elsewhere?
#if 0
	uint			modelUniformPosition;
#endif	
	union{
		struct{
			char* 			vertexPath;
			char* 			fragmentPath;
		};
		char* combiedPath;
	};
	union{
		struct{
			FILESYS::FileHandle vertexFileWriteTime;
			FILESYS::FileHandle fragmentFileWriteTime;
		};
		FILESYS::FileHandle combinedFileWriteTime;
	};
};
struct ShaderManager
{
	uint							numShaderPrograms = 0;
	uint							numUniforms = 0;
	uint							numSystemUniforms = 0;
	//index for table
	CONTAINER::StringTable<int>		shaderProgramCache;
	uint*							shaderProgramIds = NULL;
	ShaderProgram*					shaderPrograms = NULL;
	Uniform*						uniforms = NULL;
	UniformInfo*					uniformInfos = NULL;
};
static uint get_shader_program_id(const ShaderManager& manager,const char* name)
{
	int* temp = CONTAINER::access_table<int>(manager.shaderProgramCache,name);
	ASSERT_MESSAGE(temp,"FAILED TO FIND SHADERPROGRAM :: %s \n",name);
	return (uint)*temp;
}

#endif //PAKKI_SHADER_DEFS
