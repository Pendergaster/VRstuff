#ifndef PAKKI_SHADERS
#define PAKKI_SHADERS
#include <Containers.h>
#include <JsonToken.h>
#include <Utils.h>
#include <stb_image.h>
#include <glad/glad.h>
#include <ModelData.h>
#include <shader_utils.h>
#define UNIFORMTYPES(MODE)\
	MODE(INVALID)\
	MODE(INTTYPE)\
	MODE(FLOATTYPE)\
	MODE(VEC4)\
	MODE(MAT4)\
	MODE(SAMPLER2D)\
	MODE(SAMPLERCUBE)\

#define RENDERGROUPS(MODE)\
	MODE(INVALID)\
	MODE(Model)\

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
	int 	location = 0;
	char* 	name = NULL;
	int 	hashedID = 0;	
};
struct Uniform
{
	int 	type = UniformType::INVALID;
	union 
	{
		int 				_int;	
		uint				_texId;	
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
	//uint 			numTextures = 0;
	// set before rendering
	uint 			numUniforms = 0;
	//uniforms info for setting material for rendering
	UniformInfo* 	uniforms;
	ShaderType      type = ShaderType::Normal;
	RenderGroup     group = RenderGroup::INVALID;
	//TODO set this elsewhere?
	uint			modelUniformPosition;
	union{
		struct{
			char* 			vertexPath;
			char* 			fragmentPath;
		};
		char* combiedPath;
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
#define MAX_UNIFORMS 1000
#define MAX_SYSTEM_UNIFORMS 100

static inline bool setup_uniform(UniformInfo* uniform,uint id)
{
	int location = glGetUniformLocation(id,uniform->name);
	if(location == -1) return false;
	uniform->location = location;
	return true;
}
static bool compile_program(ShaderProgram* prog,uint* shaderID, char* vertSource, char* fragSource)
{
	uint vertSha = 0;
	if(!SHADER::compile_shader(GL_VERTEX_SHADER,vertSource,&vertSha))
	{
		printf("Failed to create shader %s \n",prog->vertexPath);
		return false;
	}
	uint fragSha = 0;
	if(!SHADER::compile_shader(GL_FRAGMENT_SHADER,fragSource,&fragSha))
	{
		printf("Failed to create shader %s \n",prog->fragmentPath);
		return false;
	}
	*shaderID = glCreateProgram();;
	glAttachShader(*shaderID,vertSha);
	glAttachShader(*shaderID,fragSha);
	if(!SHADER::link_shader(*shaderID,vertSha,fragSha)){
		return false;
	}
	return true;

}
static void load_shader_programs(ShaderManager* manager,CONTAINER::MemoryBlock* workingMem,CONTAINER::MemoryBlock* staticMem)
	//	ShaderProgram* programs,uint* shaderIds,int* numIds,
	//	CONTAINER::DynamicArray<char*> names,CONTAINER::StringTable<int> shaderTable,
	//	CONTAINER::MemoryBlock* staticMem,CONTAINER::MemoryBlock* workingMem,JsonToken* token,
	//	CONTAINER::DynamicArray<char*>* uniformNames)
{
	CONTAINER::DynamicArray<char*> names;
	CONTAINER::init_dynamic_array(&names);
	defer {CONTAINER::dispose_dynamic_array(&names);};
	CONTAINER::DynamicArray<char*> uniformNames;
	CONTAINER::init_dynamic_array(&uniformNames);
	defer {CONTAINER::dispose_dynamic_array(&uniformNames);};

	JsonToken token;
	token.ParseFile("importdata/shaderdata.json");
	token.GetKeys(&names);
	CONTAINER::init_table_with_block(&manager->shaderProgramCache,staticMem,names.numobj);
	manager->shaderProgramIds = (uint*)CONTAINER::get_next_memory_block(*staticMem);
	CONTAINER::increase_memory_block(staticMem,names.numobj + sizeof(uint));
	manager->shaderPrograms = (ShaderProgram*)CONTAINER::get_next_memory_block(*staticMem);
	CONTAINER::increase_memory_block(staticMem,MAX_SYSTEM_UNIFORMS * sizeof(ShaderProgram));

	manager->uniforms = (Uniform*)CONTAINER::get_next_memory_block(*staticMem);
	CONTAINER::increase_memory_block(staticMem,MAX_UNIFORMS * sizeof(Uniform));

	manager->uniformInfos = (UniformInfo*)CONTAINER::get_next_memory_block(*staticMem);
	CONTAINER::increase_memory_block(staticMem,MAX_SYSTEM_UNIFORMS * sizeof(UniformInfo));

	CONTAINER::MemoryBlock prevMemState = *workingMem;
	for(uint i = 0 ; i < names.numobj; i++)
	{
		char* currentName = names.buffer[i];
		ShaderProgram currentShaderProg;
		JsonToken* currentToken = token[currentName].GetToken();
		char* vertPath = (*currentToken)["VertexPath"].GetString();
		char* fragPath = (*currentToken)["FragmentPath"].GetString();
		char* vertFile = NULL,* fragFile = NULL;
		if(vertPath && fragPath)
		{
			ASSERT_MESSAGE(vertPath && fragPath,"PATHS NOT DEFINED FOR SHADER :: %s \n",currentName);
			vertFile = SHADER::load_shader(vertPath,workingMem);
			fragFile = SHADER::load_shader(fragPath,workingMem);
			currentShaderProg.type = ShaderType::Normal;

			currentShaderProg.vertexPath = (char*)CONTAINER::get_next_memory_block(*staticMem);
			strcpy(currentShaderProg.vertexPath , vertPath);
			CONTAINER::increase_memory_block_aligned(staticMem,(int)strlen(currentShaderProg.vertexPath) + 1);

			currentShaderProg.fragmentPath = (char*)CONTAINER::get_next_memory_block(*staticMem);
			strcpy(currentShaderProg.fragmentPath , fragPath);
			CONTAINER::increase_memory_block_aligned(staticMem,(int)strlen(currentShaderProg.fragmentPath) + 1);
		}
		else
		{
			char* combinedPath = (*currentToken)["VertFragPath"].GetString();
			ASSERT_MESSAGE(combinedPath,"PATHS NOT DEFINED FOR SHADER :: %s \n",currentName);
			SHADER::load_frag_and_vert(combinedPath,&vertFile,&fragFile,workingMem);
			currentShaderProg.type = ShaderType::Combined;

			currentShaderProg.combiedPath = (char*)CONTAINER::get_next_memory_block(*staticMem);
			strcpy(currentShaderProg.combiedPath , combinedPath);
			CONTAINER::increase_memory_block_aligned(workingMem,(int)strlen(currentShaderProg.combiedPath) + 1);
		}
		if(!compile_program(&currentShaderProg,&manager->shaderProgramIds[i],vertFile,fragFile)){
			ABORT_MESSAGE("FAILED TO COMPILE SHADER");
		}
		char* renderGroup = (*currentToken)["RenderGroup"].GetString();
		ASSERT_MESSAGE(renderGroup,"CURRENT SHADER HAS NOT DEFINED RENDERGROUP :: %s \n",
				currentName);
		for(int i3 = 0; i3 < (int)RenderGroup::MaxTypes; i3++)
		{
			if(!strcmp(renderGroup,RENDERGORUP_NAMES[i3]))		
			{
				currentShaderProg.group = (RenderGroup)i3;
				break;
			}
		}
		ASSERT_MESSAGE(currentShaderProg.group != RenderGroup::INVALID,
				"SHADER RENDER GROUP NOT DEFINED CORRECTLY :: %s \n",currentName);
		if(currentShaderProg.group == RenderGroup::Model)
		{
			int location = glGetUniformLocation(manager->shaderProgramIds[i],"model");
			ASSERT_MESSAGE(location != -1,
					"MODEL MATRIX COULD NOT BE FOUND IN :: %s \n",currentName);
			currentShaderProg.modelUniformPosition = location;
		}

		//currentShaderProg
		JsonToken* uniformToken = (*currentToken)["Uniforms"].GetToken();
		ASSERT_MESSAGE(uniformToken,"SHADER HAS NOT DEFINED UNIFORMS :: %s \n",currentName);
		uniformToken->GetKeys(&uniformNames);
		defer{uniformNames.numobj = 0;};
		currentShaderProg.numUniforms = uniformNames.numobj;
		currentShaderProg.uniforms = &manager->uniformInfos[manager->numSystemUniforms];
		ASSERT_MESSAGE(manager->numSystemUniforms + currentShaderProg.numUniforms < MAX_SYSTEM_UNIFORMS,"SYSTEM UNIFORMS IS EXEEDED");
		manager->numSystemUniforms += currentShaderProg.numUniforms;
		for(uint i2 = 0; i2 < uniformNames.numobj;i2++)
		{
			char* uniformName = uniformNames.buffer[i2];
			char* uniformTypeName = (*uniformToken)[uniformName].GetString();
			ASSERT_MESSAGE(uniformName,"UNIFORM IS NOT STRING TYPE %s in %s \n",uniformName,currentName);
			UniformInfo uniformInfo;
			uniformInfo.name = (char*)CONTAINER::get_next_memory_block(*staticMem);
			CONTAINER::increase_memory_block_aligned(staticMem,(int)strlen(uniformName) + 1);
			strcpy(uniformInfo.name,uniformName);
			uniformInfo.hashedID = CONTAINER::hash(uniformInfo.name);
			//TODO kipase location
			for(int i3 = 0; i3 < UniformType::MaxTypes; i3++)
			{
				if(!strcmp(uniformTypeName,UNIFORM_TYPE_NAMES[i3]))		
				{
					uniformInfo.type = i3;
					break;
				}
			}
			ASSERT_MESSAGE(uniformInfo.type != UniformType::INVALID,"UNIFORM %s NOT VALID TYPE IN %s",uniformName,currentName);
			if(!setup_uniform(&uniformInfo,manager->shaderProgramIds[i])){
				ABORT_MESSAGE("FAILED TO SET UP UNIFORM \n");
			}
			currentShaderProg.uniforms[i2] = uniformInfo;
		}
		manager->shaderPrograms[i] = currentShaderProg;
		*workingMem = prevMemState;
	}
}


#endif// PAKKI_SHADERS
