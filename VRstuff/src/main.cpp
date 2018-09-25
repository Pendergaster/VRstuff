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
#include "meshes.h"
#include "shaders.h"
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
#if 0
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
#endif
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

	ShaderManager shaders;
	MeshData meshes;
	TextureData textures;
	init_textures_from_metadata(&textures,&staticMemory);
	fill_mesh_cache(&meshes,&workingMemory,&staticMemory);
	load_shader_programs(&shaders,&workingMemory,&staticMemory);

	while (!glfwWindowShouldClose(window)){
		glfwPollEvents(); 
		glClearColor(1.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}
