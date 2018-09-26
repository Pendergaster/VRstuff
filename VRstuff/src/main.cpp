#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>
#undef STB_IMAGE_IMPLEMENTATION
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
// data for reloading and opengl state
#include "textures.h"
#include "meshes.h"
#include "shaders.h"
#include "glerrorcheck.h"
#include "timer.h"
#define STATIC_MEM_SIZE 100000
#define WORKING_MEM_SIZE 100000

/*
   renderings 

   for render target 
   for pass   // depth, blending?
   for shader prog
   for material 
   for mesh
   for object
   */

int main()
{
	printf("hello! \n");
	PROFILER::TimerCache timers;
	PROFILER::init_timers(&timers);
	PROFILER::start_timer(PROFILER::TimerID::Start,&timers);
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

	PROFILER::end_timer(PROFILER::TimerID::Start,&timers);
	while (!glfwWindowShouldClose(window)){
		PROFILER::start_timer(PROFILER::TimerID::Iteration,&timers);
		glfwPollEvents(); 
		glClearColor(1.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window);
		PROFILER::end_timer(PROFILER::TimerID::Iteration,&timers);
	}
	PROFILER::show_timers(&timers);
	glfwTerminate();
	return 0;
}
struct Renderer
{
	uint matrixUniformBufferObject;
	uint matrixUniformIndex;
};
#define MATRIXES_UNIFORM_LOC 0
void init_renderer(Renderer* rend,ShaderProgram* programs,
		uint* renderIds,uint numPrograms)
{
	uint* idIter = renderIds;
	for(ShaderProgram* i = programs; i < programs + numPrograms; i++,idIter++)
	{
		if(i->group != RenderGroup::Model) continue;
		uint matrixIndex = glGetUniformBlockIndex(*idIter, "MatrixBlock");   
		glUniformBlockBinding(*idIter, matrixIndex, MATRIXES_UNIFORM_LOC);
	}
		
	glGenBuffers(1, &rend->matrixUniformBufferObject);
	glBindBuffer(GL_UNIFORM_BUFFER, rend->matrixUniformBufferObject);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(MATH::mat4),
			NULL, GL_STATIC_DRAW); 
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, MATRIXES_UNIFORM_LOC, rend->matrixUniformBufferObject
			, 0, 2 * sizeof(MATH::mat4)); // bind to spot 0

	//glUniformBlockBinding(defaultRendererID, matrixIndex, 2);
}
void render()
{

}
