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
#include <MathUtil.h>
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
static MATH::vec2 mousePos(0,0);
void cursor_position_callback(GLFWwindow*, double xpos, double ypos)
{
	mousePos.x = (float)xpos;
	mousePos.y = (float)ypos;
}

static void error_callback(int e, const char *d)
{
	printf("Error %d: %s\n", e, d);
}

struct Camera 
{
	MATH::mat4	view;
	MATH::mat4	projection;
	MATH::vec3	position;
	MATH::vec3	direction;
	MATH::vec3	up;
	float		yaw;
	float		pitch;
};
static const MATH::vec3 worldUp(0.f,1.f,0.f);
static void init_camera(Camera* cam)
{
	MATH::identify(&cam->view);
	MATH::vec3 temp;
	cam->position = MATH::vec3(0.f,0.f,6.f);
	MATH::vec3 target(0.f,0.f,0.f);
	cam->direction = MATH::vec3(0.f,0.f,-1.f);

	//MATH::mat4 
	MATH::create_lookat_mat4(&cam->view,&cam->position,&cam->direction,&worldUp);
	cam->up = worldUp;
	cam->yaw = -90.0f;
	cam->pitch = 0;

	//perspective(&projection, 
	//deg_to_rad(fov), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 10000.f);

	const float fov = 45.f;
	MATH::perspective(&cam->projection,MATH::deg_to_rad * fov, 
			(float)SCREENWIDHT / (float) SCREENHEIGHT,0.1f, 10000.f);
}
static inline void update_camera(Camera* cam,MATH::vec2 newMousePos)
{
	static MATH::vec2 oldMousePos = newMousePos;
	const float sensitivity = 0.05f;

	MATH::vec2 OffVals = newMousePos - oldMousePos;
	oldMousePos = newMousePos;
	MATH::scale(&OffVals,sensitivity);
	cam->yaw += OffVals.x;
	cam->pitch -= OffVals.y;

	if(cam->pitch > 85.0f){
		cam->pitch = 85.0f;
	}
	if(cam->pitch < -85.0f){
		cam->pitch = -85.0f;
	}

	float radPitch = MATH::deg_to_rad * cam->pitch;
	float radYaw = MATH::deg_to_rad * cam->yaw;
	cam->direction.x = cosf(radPitch)	* cosf(radYaw);
	//cosf(deg_to_rad(c->pitch))*cosf(deg_to_rad(c->yaw));
	cam->direction.y = sinf(radPitch);
	//sinf(deg_to_rad(c->pitch));
	cam->direction.z = sinf(radYaw) * cosf(radPitch);
	//sinf(deg_to_rad(c->yaw))*cosf(deg_to_rad(c->pitch));
	MATH::normalize(&cam->direction);
	MATH::vec3 front = cam->position + cam->direction;
	cam->up = MATH::cross_product(cam->direction,worldUp);

	MATH::normalize(&cam->up);

	cam->up = MATH::cross_product(cam->up,cam->direction);
	MATH::normalize(&cam->up);
	MATH::create_lookat_mat4(&cam->view,&cam->position,&front,&cam->up);
	//create_lookat_mat4(&c->view, &c->cameraPos, &front, &c->camUp);

}
//TODO samaan bufferiin pojat
struct SystemUniforms
{
	uint matrixUniformBufferObject = 0;
	uint globalLightBufferObject = 0;
};
#define MATRIXES_UNIFORM_LOC 0
#define GLOBALLIGHT_UNIFORM_LOC 1
void init_systemuniforms(SystemUniforms* rend,ShaderProgram* programs,
		uint* renderIds,uint numPrograms)
{
	uint* idIter = renderIds;
	for(ShaderProgram* i = programs; i < programs + numPrograms; i++,idIter++)
	{
		//if(i->group != RenderGroup::Model) continue;
		{
			if(!BIT_CHECK(i->globalUniformFlags,GlobalUniforms::MVP)) continue;
			uint matrixIndex = glGetUniformBlockIndex(*idIter, "MatrixBlock");   
			glUniformBlockBinding(*idIter, matrixIndex, MATRIXES_UNIFORM_LOC);
			glCheckError();
		}
		{
			if(!BIT_CHECK(i->globalUniformFlags,GlobalUniforms::GlobalLight)) continue;
			uint lightIndex = glGetUniformBlockIndex(*idIter, "GlobalLight");   
			glUniformBlockBinding(*idIter, lightIndex, GLOBALLIGHT_UNIFORM_LOC);
			glCheckError();
		}
	}

	{
		glCheckError();
		glGenBuffers(1, &rend->globalLightBufferObject);

		glCheckError();
		glBindBuffer(GL_UNIFORM_BUFFER, rend->globalLightBufferObject);

		glCheckError();
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(MATH::mat4),
				NULL, GL_STATIC_DRAW); 
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glCheckError();
		glBindBufferRange(GL_UNIFORM_BUFFER, MATRIXES_UNIFORM_LOC, 
				rend->matrixUniformBufferObject, 0, 2 * sizeof(MATH::mat4)); 

		glCheckError();
	}
	{
		glCheckError();
		glGenBuffers(1, &rend->matrixUniformBufferObject);

		glCheckError();
		glBindBuffer(GL_UNIFORM_BUFFER, rend->matrixUniformBufferObject);

		glCheckError();
		glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(MATH::vec4),
				NULL, GL_STATIC_DRAW); 
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glCheckError();
		glBindBufferRange(GL_UNIFORM_BUFFER, GLOBALLIGHT_UNIFORM_LOC, 
				rend->globalLightBufferObject, 0,4 * sizeof(MATH::vec4)); 

		glCheckError();
	}

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
	for(uint i = 0 ; i < ret.numUniforms;i++)
	{
		manager->uniforms[ret.uniformIndex + i].type = manager->uniformInfos[*shaderIndex].type;
	}
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
static inline void set_material_texture(ShaderManager* manager,
		Material* mat,int index, const int shaderCacheID)
{
	Uniform* uni = get_uniform(manager,mat,index);
	ASSERT_MESSAGE(uni->type == UniformType::SAMPLER2D,"UNIFORM IS NOT CORRECT TYPE\n");
	uni->_textureCacheId = shaderCacheID;
}
//TODO textuuri set BOE BOE 
//TODO camera 
struct RenderData
{
	Material			material;
	MeshId				meshID = 0;
	MATH::vec3			position;
	MATH::vec3			oriTemp;
	MATH::quaternion	orientation;
	float				scale = 0;
};

void render(RenderData* renderables,int numRenderables,
		MeshData* meshes,ShaderManager* shaders ,
		const SystemUniforms* uniforms,const Camera* camera,uint* textureIds);

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
	glViewport(0, 0, SCREENWIDHT, SCREENHEIGHT);
	glfwSetErrorCallback(error_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

	CONTAINER::MemoryBlock staticMemory;
	CONTAINER::init_memory_block(&staticMemory,STATIC_MEM_SIZE);
	CONTAINER::MemoryBlock workingMemory;
	CONTAINER::init_memory_block(&workingMemory,WORKING_MEM_SIZE);

	ShaderManager shaders;
	MeshData meshes;
	TextureData textures;
	init_textures_from_metadata(&textures,&staticMemory);
	fill_mesh_cache(&meshes,&workingMemory,&staticMemory);

	glCheckError();
	load_shader_programs(&shaders,&workingMemory,&staticMemory);

	glCheckError();
	Camera camera;
	init_camera(&camera);


	SystemUniforms sysUniforms;
	glCheckError();
	init_systemuniforms(&sysUniforms,shaders.shaderPrograms,
			shaders.shaderProgramIds,shaders.numShaderPrograms);
	PROFILER::end_timer(PROFILER::TimerID::Start,&timers);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	const double dt = 1.0 / 60.0;
	double currentTime = glfwGetTime();
	double accumulator = 0.0;

	Material material = create_new_material(&shaders,"MainProg");
	int moonTex = get_texture(textures,"MoonTexture");
	set_material_texture(&shaders,&material,0,moonTex);
	MeshId meshId = get_mesh(&meshes,"Planet");
	RenderData renderData;

	renderData.material = material;
	renderData.meshID = meshId;
	renderData.oriTemp = MATH::vec3(0,0,0);
	renderData.position = MATH::vec3(0,0,0);
	renderData.scale = 1;
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	while (!glfwWindowShouldClose(window))
	{
		PROFILER::start_timer(PROFILER::TimerID::Iteration,&timers);
		double newTime = glfwGetTime();
		double frameTime = newTime - currentTime;
		currentTime = newTime;
		accumulator += frameTime;
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		{
			break;
		}
		glfwPollEvents(); 
		while(accumulator >= dt)
		{
			update_camera(&camera,mousePos);
			printf("pitch %f : yaw %f \n",camera.pitch,camera.yaw);
			accumulator -= dt;
		}
		glClearColor(1.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_CULL_FACE);  
		//void render(RenderData* renderables,int numRenderables,
		//	MeshData* meshes,ShaderManager* shaders ,
		//		const SystemUniforms* uniforms,const Camera* camera,uint* textureIds);
		render(&renderData,1,&meshes,&shaders,&sysUniforms,&camera,textures.textureIds);
		glCheckError();

		glfwSwapBuffers(window);
		PROFILER::end_timer(PROFILER::TimerID::Iteration,&timers);
	}
	PROFILER::show_timers(&timers);
	glfwTerminate();
	return 0;
}
void render(RenderData* renderables,int numRenderables,
		MeshData* meshes,ShaderManager* shaders ,
		const SystemUniforms* uniforms,const Camera* camera,uint* textureIds)
{
	glBindBuffer(GL_UNIFORM_BUFFER,uniforms->matrixUniformBufferObject);
	glBufferSubData(GL_UNIFORM_BUFFER,0,sizeof(MATH::mat4) * 2, (void*)camera);
	glBindBuffer(GL_UNIFORM_BUFFER,0);

	struct lightutil{
		MATH::vec4 dir;
		MATH::vec4 ambient;
		MATH::vec4 diffuse;
		MATH::vec4 specular;
	} light;
	light.dir = MATH::vec4(-0.2f, -1.0f, -0.3f,1.f);
	light.ambient = MATH::vec4(0.05f, 0.05f, 0.05f,1.f);
	light.diffuse = MATH::vec4(0.4f, 0.4f, 0.4f,1.f);
	light.specular = MATH::vec4( 0.5f, 0.5f, 0.5f,1.f);
	glBindBuffer(GL_UNIFORM_BUFFER,uniforms->globalLightBufferObject);
	glBufferSubData(GL_UNIFORM_BUFFER,0,sizeof(MATH::vec4) * 2, (void*)camera);
	glBindBuffer(GL_UNIFORM_BUFFER,0);

	glCheckError();
	for(RenderData* i = renderables; i < renderables + numRenderables; i++)
	{
		ShaderProgram* prog = &shaders->shaderPrograms[i->material.shaderProgram];
		uint glID = shaders->shaderProgramIds[i->material.shaderProgram];
		glUseProgram(glID);
		MATH::mat4 model; // TODO SET THIS 
		MATH::identify(&model);
		MATH::translate(&model,i->position);
		MATH::rotateY(&model,i->oriTemp.y);
		MATH::rotateZ(&model,i->oriTemp.z);
		MATH::rotateX(&model,i->oriTemp.x);

		MATH::scale_mat4(&model,i->scale);
		glUniformMatrix4fv(prog->modelUniformPosition, 1, GL_FALSE, (GLfloat*)model.mat);
		glCheckError();
		for(uint i2 = 0; i2 < i->material.numUniforms;i2++)
		{
			Uniform* uniToSet = &shaders->uniforms[i->material.uniformIndex + i2];
			UniformInfo* info = &prog->uniforms[i2];

			//for(int texPlace = 0; texPlace < prog->)
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
						glActiveTexture(GL_TEXTURE0 + info->glTexLocation);
						glBindTexture(GL_TEXTURE_2D, textureIds[uniToSet->_textureCacheId]);
					}break;
				case UniformType::INVALID: default:
					{
						ABORT_MESSAGE("INVALID UNIFORM TYPE");
					}break;
			}
		}

		glCheckError();
		//SHADER::set_mat4_name();
		Mesh* currentMesh = &meshes->meshArray[i->meshID];
		MeshInfo* currentMeshInfo = &meshes->meshInfos[i->meshID];
		glBindVertexArray(currentMesh->vao);
		glDrawElements(GL_TRIANGLES,currentMeshInfo->numIndexes, GL_UNSIGNED_INT,0);
		glCheckError();
	}
}

