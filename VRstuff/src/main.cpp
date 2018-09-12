#define _CRT_SECURE_NO_WARNINGS
#include <glad/glad.h>
#include "glad.c"
#include <glwf3/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#define SCREENWIDHT 800
#define SCREENHEIGHT 1000
#include<Utils.h>
#include<shader_utils.h>
#include<Containers.h>

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
	//TODO set this
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
	uint numMeshes = 0;
	uint numShaderPrograms = 0;
	////index for table
	//index for table
	CONTAINER::StringTable<int> meshCache;
	CONTAINER::StringTable<int> shaderProgramCache;
		

};

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow* window = glfwCreateWindow(SCREENWIDHT,SCREENHEIGHT, "Tabula Rasa", NULL, NULL);
	ASSERT_MESSAGE(window,"FAILED TO INIT WINDOW \n"); // failed to create window
	glfwMakeContextCurrent(window);
	ASSERT_MESSAGE((gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)),"FAILED TO INIT GLAD");

	CONTAINER::StaticAllocator StaticMemory;
	CONTAINER::init_static_memory;

	while (!glfwWindowShouldClose(window)){
		glfwPollEvents(); 
		glClearColor(1.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}
