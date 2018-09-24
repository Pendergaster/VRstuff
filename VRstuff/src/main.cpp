#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <malloc.h>
#include <math.h>
#include <cstring>
#include <glad/glad.h>
#include "glad.c"
#include <glwf3/glfw3.h>
#define SCREENWIDHT 800
#define SCREENHEIGHT 1000
#include<JsonToken.h>
#include<ModelData.h>
#include<shader_utils.h>
#include<Utils.h>
#include<Containers.h>
#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>
#undef STB_IMAGE_IMPLEMENTATION
// data for reloading and opengl state
#include "textures.h"
#define UNIFORMTYPES(MODE)\
	MODE(INVALID)\
	MODE(INTTYPE)\
	MODE(FLOATTYPE)\
	MODE(VEC4)\
	MODE(MAT4)\
	MODE(SAMPLER2D)\
	MODE(SAMPLERCUBE)\

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
		TextureID			_texId;	
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
struct ShaderProgram
{
	// used for setting up material
	uint 			numTextures = 0;
	// set before rendering
	uint 			numUniforms = 0;
	//uniforms info for setting material for rendering
	UniformInfo* 	uniforms;
	ShaderType      type = ShaderType::Normal;
	union{
		struct{
			char* 			vertexPath;
			char* 			fragmentPath;
		};
		char* combiedPath;
	};

};
struct RenderingData
{
	uint							numShaderPrograms = 0;
	//index for table
	CONTAINER::StringTable<int>		shaderProgramCache;
	uint*							shaderPrograms = NULL;
};
/*
   {
   "Name" :
   { 
rawpath : "path",
metapath : "path",
},
"Name" : "path",
"Name" : "path",
"Name" : "path",
}
*/
//TODO inlinaa tämä
static void load_shader_programs(ShaderProgram* programs,uint* shaderIds,int* numIds,
		CONTAINER::DynamicArray<char*> names,CONTAINER::StringTable<int> shaderTable,
		CONTAINER::MemoryBlock* staticMem,CONTAINER::MemoryBlock* workingMem,JsonToken* token,
		CONTAINER::DynamicArray<char*>* uniformNames)
{
	int num = 0;
	for(uint i = 0 ; i < names.numobj; i++)
	{
		char* currentName = names.buffer[i];
		ShaderProgram currentShaderProg;
		JsonToken* currentToken = (*token)[currentName].GetToken();
		char* vertPath = (*currentToken)["VertexPath"].GetString();
		char* fragPath = (*currentToken)["FragmentPath"].GetString();
		char* vertFile = NULL,* fragFile = NULL;
		CONTAINER::MemoryBlock prevMemState = *workingMem;
		if(vertPath && fragPath)
		{
			ASSERT_MESSAGE(vertPath && fragPath,"PATHS NOT DEFINED FOR SHADER :: %s \n",currentName);
			vertFile = SHADER::load_shader(vertPath,workingMem);
			vertFile = SHADER::load_shader(fragPath,workingMem);
			currentShaderProg.type = ShaderType::Normal;

			currentShaderProg.vertexPath = (char*)CONTAINER::get_next_memory_block(*staticMem);
			strcpy(currentShaderProg.vertexPath , vertFile);
			CONTAINER::increase_memory_block_aligned(workingMem,strlen(currentShaderProg.vertexPath) + 1);

			currentShaderProg.fragmentPath = (char*)CONTAINER::get_next_memory_block(*staticMem);
			strcpy(currentShaderProg.fragmentPath , fragFile);
			CONTAINER::increase_memory_block_aligned(workingMem,strlen(currentShaderProg.fragmentPath) + 1);

		}
		else
		{
			char* combinedPath = (*currentToken)["VertFragPath"].GetString();
			ASSERT_MESSAGE(combinedPath,"PATHS NOT DEFINED FOR SHADER :: %s \n",currentName);
			SHADER::load_frag_and_vert(combinedPath,&vertFile,&fragFile,workingMem);
			currentShaderProg.type = ShaderType::Combined;

			currentShaderProg.combiedPath = (char*)CONTAINER::get_next_memory_block(*staticMem);
			strcpy(currentShaderProg.combiedPath , combinedPath);
			CONTAINER::increase_memory_block_aligned(workingMem,strlen(currentShaderProg.combiedPath) + 1);
		}
		//currentShaderProg
		JsonToken* uniformToken = (*currentToken)["Uniforms"].GetToken();
		ASSERT_MESSAGE(uniformToken,"SHADER HAS NOT DEFINED UNIFORMS :: %s \n",currentName);
		uniformToken->GetKeys(uniformNames);
		for(uint i2 = 0; i2 < uniformNames->numobj;i2++)
		{
			char* uniformName = uniformNames->buffer[i2];
			char* uniformTypeName = (*uniformToken)[uniformName].GetString();
			ASSERT_MESSAGE(uniformName,"UNIFORM IS NOT STRING TYPE %s in %s \n",uniformName,currentName);
			UniformInfo uniform;
			for(int i3 = 0; i3 < UniformType::MaxTypes; i3++)
			{
				if(!strcmp(uniformTypeName,UNIFORM_TYPE_NAMES[i3]))		
				{
					uniform.type = i3;
					break;
				}
			}
			ASSERT_MESSAGE(uniform.type != UniformType::INVALID,"UNIFORM %s NOT VALID TYPE IN %s",uniformName,currentName);
		}
		*workingMem = prevMemState;
	}
}
static void init_meshes()
{



}
static void init_rendering_data(RenderingData* rend,
		CONTAINER::MemoryBlock* staticMem,CONTAINER::MemoryBlock* workingMem)
{
	JsonToken meshToken;
	JsonToken shaderToken;
	JsonToken textureToken;
	CONTAINER::DynamicArray<char*> meshNames;
	CONTAINER::init_dynamic_array(&meshNames);
	defer {CONTAINER::dispose_dynamic_array(&meshNames);};
	CONTAINER::DynamicArray<char*> shaderNames;
	CONTAINER::init_dynamic_array(&shaderNames);
	defer {CONTAINER::dispose_dynamic_array(&shaderNames);};
	CONTAINER::DynamicArray<char*> textureNames;
	CONTAINER::init_dynamic_array(&textureNames);
	defer {CONTAINER::dispose_dynamic_array(&textureNames);};

	meshToken.ParseFile("importdata/meshdata.json");
	shaderToken.ParseFile("importdata/shaderdata.json");
	textureToken.ParseFile("importdata/textureToken.json");

	meshToken.GetKeys(&meshNames);
	shaderToken.GetKeys(&shaderNames);
	textureToken.GetKeys(&textureNames);

	CONTAINER::init_table_with_block<int>(&rend->meshCache,
			staticMem,meshNames.numobj);
	CONTAINER::init_table_with_block<int>(&rend->shaderProgramCache,
			staticMem,shaderNames.numobj);
	CONTAINER::init_table_with_block<int>(&rend->textureCache,
			staticMem,textureNames.numobj);
	rend->meshArray = (Mesh*)CONTAINER::get_next_memory_block(*staticMem);
	CONTAINER::increase_memory_block(staticMem,sizeof(Mesh) * meshNames.numobj);
	rend->textureIds = (uint*)CONTAINER::get_next_memory_block(*staticMem);
	CONTAINER::increase_memory_block(staticMem,sizeof(uint) * textureNames.numobj);
	rend->textureInfos = (TextureInfo*)CONTAINER::get_next_memory_block(*staticMem);
	CONTAINER::increase_memory_block(staticMem,sizeof(TextureInfo) * textureNames.numobj);


	int numMeshes = 0;
	fill_mesh_cache(
			rend->meshArray,&numMeshes, &rend->meshCache,
			&meshToken,&meshNames,workingMem,staticMem);
	int numTextures = 0;
	load_textures(&textureToken,rend->textureIds,rend->textureInfos,&numTextures,&rend->textureCache,textureNames,staticMem);
}

#define STATIC_MEM_SIZE 10000
#define WORKING_MEM_SIZE 100000
int main()
{
	printf("hello! \n");
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow* window = glfwCreateWindow(SCREENWIDHT,SCREENHEIGHT, "Tabula Rasa", NULL, NULL);
	ASSERT_MESSAGE(window,"FAILED TO INIT WINDOW \n"); // failed to create window
	glfwMakeContextCurrent(window);
	ASSERT_MESSAGE((gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)),"FAILED TO INIT GLAD");

	CONTAINER::MemoryBlock staticMemory;
	CONTAINER::init_memory_block(&staticMemory,STATIC_MEM_SIZE);
	CONTAINER::MemoryBlock workingMemory;
	CONTAINER::init_memory_block(&workingMemory,WORKING_MEM_SIZE);


	RenderingData rendData;
	init_rendering_data(&rendData,&staticMemory,&workingMemory);

	while (!glfwWindowShouldClose(window)){
		glfwPollEvents(); 
		glClearColor(1.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}
