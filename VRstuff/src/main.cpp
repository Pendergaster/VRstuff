#define _CRT_SECURE_NO_WARNINGS
#include <glad/glad.h>
#include "glad.c"
#include <glwf3/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#define SCREENWIDHT 800
#define SCREENHEIGHT 1000
#include<Utils.h>
#include<Containers.h>
#include<shader_utils.h>
#include<JsonToken.h>
#include<ModelData.h>
typedef int TextureID;

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
		Max
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

struct ShaderProgram
{
	char* 			vertexPath = NULL;
	char* 			fragmentPath = NULL;
	// used for setting up material
	uint 			numTextures = 0;
	// set before rendering
	uint 			numUniforms = 0;
	//uniforms info for setting material for rendering
	UniformInfo* 	uniforms;
};

struct Mesh
{
	int vertBuffer = 0;
	int fragmentBuffer = 0;
	int indexBuffer = 0;
};

struct RenderingData
{
	uint							numMeshes = 0;
	uint							numShaderPrograms = 0;
	//index for table
	CONTAINER::StringTable<int>		meshCache;
	CONTAINER::StringTable<int>		shaderProgramCache;
	CONTAINER::StringTable<int>		textureCache;
	Mesh*							meshArray;

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
static void fill_mesh_cache(Mesh* meshArray,CONTAINER::StringTable<int>* nameTable,JsonToken* token,
		CONTAINER::DynamicArray<char*>* names,CONTAINER::MemoryBlock* workingMem,CONTAINER::MemoryBlock* StaticAllocator)
{
	for(uint i = 0; i < names->numobj;i++)
	{
		char* currentName = names->buffer[i];
		JsonToken* meshToken = (*token)[currentName].GetToken();
		ASSERT_MESSAGE(meshToken , "MESH TOKEN NOT VALID %s \n",currentName);
		char* metaDataPath = (*meshToken)["metaDataPath"].GetString();
		printf("metaDataPath path for %s is %s \n",currentName,metaDataPath);
		size_t sizeOfFile = 0;
		void* modelDataDump = FILESYS::load_binary_file_to_block(metaDataPath,workingMem,&sizeOfFile);
		ModelAligment* aligment = (ModelAligment*)modelDataDump;		
		modelDataDump = (void*)((int8*)modelDataDump + sizeof(ModelAligment));
		ModelData modelData;
		modelData.vertexes = (MATH::vec3*)modelDataDump;
		modelDataDump = (void*)((int8*)modelDataDump + sizeof(MATH::vec3) * aligment->numVerts);
		modelData.normals = (MATH::vec3*)modelDataDump;
		modelDataDump = (void*)((int8*)modelDataDump + sizeof(MATH::vec3) * aligment->numIndexes);
		modelData.indexes = (int*)modelDataDump;
	}
}
static void init_rendering_data(RenderingData* rend,
		CONTAINER::MemoryBlock* staticMem)
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
	fill_mesh_cache(rend->meshArray,&rend->meshCache,&meshToken,&meshNames);


}

#define STATIC_MEM_SIZE 10000
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

	RenderingData rendData;
	init_rendering_data(&rendData,&staticMemory);

	while (!glfwWindowShouldClose(window)){
		glfwPollEvents(); 
		glClearColor(1.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}
