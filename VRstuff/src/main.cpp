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
struct SystemUniforms
{
	uint matrixUniformBufferObject;
	//uint matrixUniformIndex;
};

struct Camera 
{
	MATH::mat4 view;
	MATH::mat4 projection;
};
#define MATRIXES_UNIFORM_LOC 0
void init_systemuniforms(SystemUniforms* rend,ShaderProgram* programs,
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

	glBindBufferRange(GL_UNIFORM_BUFFER, MATRIXES_UNIFORM_LOC, 
			rend->matrixUniformBufferObject, 0, 2 * sizeof(MATH::mat4)); 

	//glUniformBlockBinding(defaultRendererID, matrixIndex, 2);
}

struct Material
{
	uint shaderProgram;
	uint uniformIndex;
	uint numUniforms;
};
Material create_new_material(ShaderManager* manager,const char* name)
{
	int* shaderIndex = CONTAINER::access_table(manager->shaderProgramCache,name);
	ASSERT_MESSAGE(shaderIndex,"SHADERPROGRAM DOES NOT EXIST :: %s \n",name);
	ShaderProgram* program = &manager->shaderPrograms[*shaderIndex];
	Material ret;
	ret.uniformIndex =  manager->numUniforms;
	manager->numUniforms += program->numUniforms;
	ret.numUniforms = program->numUniforms;
	ret.shaderProgram = *shaderIndex;
	return ret;
}
static inline Uniform* get_uniform(ShaderManager* manager,
		Material* mat,int index)
{
	Uniform* m = &manager->uniforms[mat->uniformIndex + index];
	return m;
}
static inline void set_material_vec4(ShaderManager* manager,
		Material* mat,int index, const MATH::vec4& vec)
{
	Uniform* uni = get_uniform(manager,mat,index);
	ASSERT_MESSAGE(uni->type == UniformType::VEC4,"UNIFORM IS NOT CORRECT TYPE\n");
	uni->_vec4 = vec;
}
static inline void set_material_float(ShaderManager* manager,
		Material* mat,int index, const float val)
{
	Uniform* uni = get_uniform(manager,mat,index);
	ASSERT_MESSAGE(uni->type == UniformType::FLOATTYPE,"UNIFORM IS NOT CORRECT TYPE\n");
	uni->_float = val;
}
static inline void set_material_int(ShaderManager* manager,
		Material* mat,int index, const int val)
{
	Uniform* uni = get_uniform(manager,mat,index);
	ASSERT_MESSAGE(uni->type == UniformType::INTTYPE,"UNIFORM IS NOT CORRECT TYPE\n");
	uni->_int = val;
}
//TODO textuuri set BOE BOE 
//TODO camera 
struct RenderData
{
	Material			material;
	uint				meshID = 0;
	MATH::vec3			pos;
	MATH::vec3			oriTemp;
	MATH::quaternion	orientation;
};
void render(RenderData* renderables,int numRenderables,
		MeshData* meshes,ShaderManager* shaders ,
		const SystemUniforms* uniforms,const Camera* camera,uint* textureIds)
{
	glBindBuffer(GL_UNIFORM_BUFFER,uniforms->matrixUniformBufferObject);
	glBufferSubData(GL_UNIFORM_BUFFER,0,sizeof(MATH::mat4) * 2, (void*)camera);
	glBindBuffer(GL_UNIFORM_BUFFER,0);

	for(RenderData* i = renderables; i < renderables + numRenderables; i++)
	{
		ShaderProgram* prog = &shaders->shaderPrograms[i->material.shaderProgram];
		uint glID = shaders->shaderProgramIds[i->material.shaderProgram];
		glUseProgram(glID);
		MATH::mat4 model; // TODO SET THIS 
		for(uint i2 = 0; i2 < i->material.numUniforms;i2++)
		{
			Uniform* uniToSet = &shaders->uniforms[i->material.uniformIndex + i2];
			UniformInfo* info = &prog->uniforms[i2];
			for(int texPlace = 0; texPlace < prog->)
			//	prog->uniforms[i2].type
			switch(info->type)
			{
				case UniformType::VEC4:
					{
						glUniform4f(info->location,uniToSet->_vec4.x,
								uniToSet->_vec4.y,uniToSet->_vec4.z,uniToSet->_vec4.w);
					}break;
				case UniformType::MAT4:
					{
						glUniformMatrix4fv(info->location,1,GL_FALSE,
								(GLfloat*)uniToSet->_mat4.mat);
					}break;
				case UniformType::FLOATTYPE:
					{
						glUniform1f(info->location,uniToSet->_float);
					}break;
				case UniformType::INTTYPE:
					{
						glUniform1i(info->location,uniToSet->_int);
					}break;
				case UniformType::SAMPLER2D:
					{
						
					}break;
				case UniformType::INVALID: default:
					{
						ABORT_MESSAGE("INVALID UNIFORM TYPE");
					}break;


			}
		}
		//SHADER::set_mat4_name();

	}
}

