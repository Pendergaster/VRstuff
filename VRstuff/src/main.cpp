#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#define VR 1
#if VR
#include <dxgi.h>
#endif
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

#if 1
#include <imgui/imgui.h>
#include <imgui/imgui.cpp>
#include <imgui/imgui_draw.cpp>
#include <imgui/imgui_widgets.cpp>
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui_impl_glfw.h"
#include "imgui_impl_glfw.cpp"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_opengl3.cpp"
#endif

#define SCREENWIDHT 1000
#define SCREENHEIGHT 1000
//#include <file_system.h>
#include<JsonToken.h>
#include <MathUtil.h>
#include<ModelData.h>
#include<shader_utils.h>
#include<Utils.h>
#include<Containers.h>
#include "input.h"
// data for reloading and opengl state
#include "textures.h"
#include "meshes.h"
#include "shaders.h"
#include "glerrorcheck.h"
#include "timer.h"
#include "SystemUniforms.h"
#include "input.h"
#if 1
#include <glm/matrix.hpp>
#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>
#endif
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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//printf("reee\n");
	if (action == GLFW_PRESS)
	{
		INPUTS::set_key_down(key);
	}
	else if (action == GLFW_RELEASE)
	{
		INPUTS::set_key_up(key);
	}
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
#if !VR
	MATH::perspective(&cam->projection,MATH::deg_to_rad * fov, 
			(float)SCREENWIDHT / (float) SCREENHEIGHT,0.1f, 10000.f);
#else 
		MATH::perspective(&cam->projection,MATH::deg_to_rad * fov, 
			(float)(SCREENWIDHT/2) / (float) SCREENHEIGHT,0.1f, 10000.f);
#endif
}
static inline void update_camera(Camera* cam,MATH::vec2 newMousePos)
{
	static MATH::vec2 oldMousePos = newMousePos;
	const float sensitivity = 0.00005f;

	MATH::vec2 OffVals = newMousePos - oldMousePos;
	oldMousePos = newMousePos;
	MATH::scale(&OffVals,sensitivity);
	cam->yaw += OffVals.x;
	cam->pitch -= OffVals.y;
#if 0
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
#endif

}
//TODO samaan bufferiin pojat


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
		manager->uniforms[ret.uniformIndex + i].type = manager->uniformInfos
			[ret.uniformIndex + i].type;
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

//void render(RenderData* renderables,int numRenderables,
//MeshData* meshes,ShaderManager* shaders ,
//const SystemUniforms* uniforms,const Camera* camera,uint* textureIds);

#if 0
uint frameBuffer = 0;
uint renderBuffer;
uint eyeTextures[2];
#endif
enum FrameBufferAttacment : int
{
	None = 1 << 0,
	Color = 1 << 1,
	Depth = 1 << 2,
};
struct FrameTexture
{
	uint texture;
	uint buffer;
	uint textureWidth;
	uint textureHeight;
	int	 attachments;
};
static inline FrameTexture create_new_frameTexture(uint width,uint height,GLenum attachment,int type)
{
	FrameTexture ret;
	ret.textureWidth = width;
	ret.textureHeight = height;
	glGenFramebuffers(1,&ret.buffer);
	glBindFramebuffer(GL_FRAMEBUFFER,ret.buffer);
	glGenTextures(1,&ret.texture);
	glBindTexture(GL_TEXTURE_2D,ret.texture);
	//TODO dimensiot oculucselta
	//call glViewport before rendering to new size of the oculus!!!
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,NULL);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D,0);
	glFramebufferTexture2D(GL_FRAMEBUFFER,attachment,GL_TEXTURE_2D,ret.texture,0);
	glCheckError();
	if(BIT_CHECK(type,FrameBufferAttacment::Depth))
	{
		uint rbo = 0;
		glGenRenderbuffers(1,&rbo);
		glBindRenderbuffer(GL_RENDERBUFFER,rbo);
		//TODO tähän vr dimensiot
		glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,ret.textureWidth,ret.textureHeight);
		glBindRenderbuffer(GL_RENDERBUFFER,0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,rbo);
	}
	glCheckError();
	if(!(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)){
		ABORT_MESSAGE("FAILED TO SET FRAMEBUFFER \n");
	}
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	ret.attachments = type;
	return ret;
}
static inline void set_and_clear_frameTexture(const FrameTexture& frameTex)
{
	glCheckError();
	glBindFramebuffer(GL_FRAMEBUFFER,frameTex.buffer);
	glClearColor(0.f,0.f,0.f,1.f);
	if(BIT_CHECK(frameTex.attachments,FrameBufferAttacment::Depth) && BIT_CHECK(frameTex.attachments,FrameBufferAttacment::Color)  ) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	else if BIT_CHECK(frameTex.attachments,FrameBufferAttacment::Depth) {
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	else if BIT_CHECK(frameTex.attachments,FrameBufferAttacment::Color) {
		glClear(GL_COLOR_BUFFER_BIT);
	}
	glViewport(0, 0, frameTex.textureWidth, frameTex.textureHeight);
	glCheckError();
}
void render(RenderData* renderables,int numRenderables,
		MeshData* meshes,ShaderManager* shaders ,
		const SystemUniforms* uniforms,const Camera* camera,uint* textureIds,
		FrameTexture* frameTextures,uint* eyeVaos,Material vrProgramMaterial);

GLFWwindow* window = NULL;
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
	window = glfwCreateWindow(SCREENWIDHT,SCREENHEIGHT, "Tabula Rasa", NULL, NULL);
	ASSERT_MESSAGE(window,"FAILED TO INIT WINDOW \n"); // failed to create window
	glfwMakeContextCurrent(window);
	ASSERT_MESSAGE((gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)),"FAILED TO INIT GLAD");
	glViewport(0, 0, SCREENWIDHT, SCREENHEIGHT);

	glfwSetErrorCallback(error_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetKeyCallback(window, key_callback);

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

	LOG("Game inited \n"); fflush(stdout);
	glCheckError();
	Camera camera;
	init_camera(&camera);


	SystemUniforms sysUniforms;
	glCheckError();
	init_systemuniforms(&sysUniforms,shaders.shaderPrograms,
			shaders.shaderProgramIds,shaders.numShaderPrograms);



	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	const char* glsl_version = "#version 130";
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Setup style
	ImGui::StyleColorsDark();
	INPUTS::Input inputs;
	init_inputs(&inputs);

	PROFILER::end_timer(PROFILER::TimerID::Start,&timers);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
	//renderData.oriTemp = MATH::vec3(0,0,0);
	//renderData.orientation = MATH::quaternion(0,0,0,0);
	renderData.position = MATH::vec3(0,0,0);
	renderData.scale = 0.7f;
#if 0
	{
		glGenFramebuffers(1,&frameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER,frameBuffer);


		glGenTextures(2,eyeTextures);
		for(int i = 0; i < 2;i++)
		{
			glBindTexture(GL_TEXTURE_2D,eyeTextures[i]);
			//TODO dimensiot oculucselta
			//call glViewport before rendering to new size of the oculus!!!
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,SCREENWIDHT,SCREENHEIGHT,0,GL_RGB,GL_UNSIGNED_BYTE,NULL);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D,0);
			glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,eyeTextures[i],0);

		}
		glGenRenderbuffers(1,&renderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER,renderBuffer);
		//TODO tähän vr dimensiot
		glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,SCREENWIDHT,SCREENHEIGHT);
		glBindRenderbuffer(GL_RENDERBUFFER,0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,renderBuffer);
		if(!(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)){
			ABORT_MESSAGE("FAILED TO SET FRAMEBUFFER \n");
		}
		glBindFramebuffer(GL_FRAMEBUFFER,0);

	}
#endif
#if 0
	{// otetaan depth ja stencil?
		uint renderBuffer;
		glGenRenderbuffers(1,renderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER,renderBuffer);
		//TODO tähän vr dimensiot
		glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,SCREENWIDHT,SCREENHEIGHT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,renderBuffer);

	};
#endif

	FrameTexture eyes[2] = 
	{create_new_frameTexture(SCREENWIDHT/ 2,SCREENHEIGHT,GL_COLOR_ATTACHMENT0,FrameBufferAttacment::Color |FrameBufferAttacment::Depth  )
		,create_new_frameTexture(SCREENWIDHT/ 2,SCREENHEIGHT,GL_COLOR_ATTACHMENT0,FrameBufferAttacment::Color |FrameBufferAttacment::Depth )};

	glBindFramebuffer(GL_FRAMEBUFFER,0);
	//static inline FrameTexture create_new_frameTexture(uint width,uint height,GLenum attachment,FrameBufferAttacment type)

	uint vrVaos[2];
	{

		float verticesLeft[] = {
			0.0f,  1.f,  1.f , 1.f,		// top right
			0.0f, -1.f,  1.f , 0.f,		// bottom right
			-1.0f, -1.0f, 0.f , 0.f,		// bottom left
			-1.0f,  1.0f, 0.f , 1.f		// top left 
		};
		float verticesRight[] = {
			1.f,  1.f,  1.f , 1.f,		// top right
			1.f, -1.f,  1.f , 0.f,		// bottom right
			0.0f, -1.f, 0.f , 0.f,		// bottom left
			0.0f,  1.f,  0.f , 1.f		// top left 
		};

		unsigned int indices[] = {  // note that we start from 0!
			0, 1, 3,				// first triangle
			1, 2, 3					// second triangle
		};  

		uint buffers[4];
		glGenBuffers(4,buffers);
		glGenVertexArrays(2,vrVaos);
		float* verts[2] = {verticesLeft,verticesRight};
		for(int i = 0; i < 2; i++)
		{
			glBindVertexArray(vrVaos[i]);

			glBindBuffer(GL_ARRAY_BUFFER,buffers[i * 2]);
			glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,
					2 * sizeof(float) + 2* sizeof(float),(void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,
					2 * sizeof(float) + 2* sizeof(float),(void*)(2 * sizeof(float)));
			glEnableVertexAttribArray(1);
			glBufferData(GL_ARRAY_BUFFER,sizeof(float)*(4*4),NULL,GL_STATIC_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(float)*(4 * 4),verts[i]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,buffers[i * 2 + 1]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 6 * sizeof(uint), indices);

		}
		glCheckError();
	}

	//ShaderProgram vrProgram = get_shader_program(shaders,"EyeProg");
	//uint vrProgramID = get_shader_program_id(shaders,"EyeProg");
	Material vrMaterial = create_new_material(&shaders,"EyeProg");
	set_material_texture(&shaders,&vrMaterial,0,0);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	while (!glfwWindowShouldClose(window))
	{

		glfwPollEvents();
#if 0
		int joyStickPresent = glfwJoystickPresent(GLFW_JOYSTICK_1);
		//printf("%d \n",joyStickPresent);
		if(1 == joyStickPresent)
		{
			int countAxes = 0;
			const char* name = glfwGetJoystickName(GLFW_JOYSTICK_1);
			printf("%s \n",name);
			const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1,&countAxes);
			//printf("%d \n",countAxes);
			int buttonCount = 0;
			const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1,&buttonCount);
			//printf("%d \n",buttonCount);
			for(unsigned char* i = (unsigned char*)buttons; i < buttons + buttonCount ; i++)
			{
				if(GLFW_PRESS == *i)
				{
					printf("%d \n",(int)(i - buttons));
				}
			}
		}
#endif
		PROFILER::start_timer(PROFILER::TimerID::Iteration,&timers);
		double newTime = glfwGetTime();
		double frameTime = newTime - currentTime;
		currentTime = newTime;
		accumulator += frameTime;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		{
			//static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			static bool show = false;
			static bool show2 = false;
			ImGui::Checkbox("Demo Window", &show);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show2);

			ImGui::SliderFloat("pitch", &renderData.oriTemp.x, 0.0f, 360.0f);
			ImGui::SliderFloat("yaw", &renderData.oriTemp.y, 0.0f, 360.0f);
			ImGui::SliderFloat("roll", &renderData.oriTemp.z, 0.0f, 360.0f);
			ImGui::SliderFloat("posx", &renderData.position.x, 0.0f, 10.0f);
			ImGui::SliderFloat("posy", &renderData.position.y, 0.0f, 10.0f); 
			ImGui::SliderFloat("posz", &renderData.position.z, 0.0f, 10.0f); 
			//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
			if (show)
			{
				ImGui::Begin("Another Window", &show);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
				ImGui::Text("Hello from another window!");
				if (ImGui::Button("Close Me"))
					show = false;
				ImGui::End();
			}
		}

		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		{
			break;
		}
		//glfwPollEvents(); 
		while(accumulator >= dt)
		{
			update_camera(&camera,mousePos);
			if(INPUTS::key_pressed(INPUTS::Key::KEY_W))
			{
				printf("KEY W \n");
			}
			if(INPUTS::key_pressed(INPUTS::Key::KEY_A))
			{
				printf("KEY A \n");
			}
			if(INPUTS::key_pressed(INPUTS::Key::KEY_D))
			{
				printf("KEY D \n");
			}
			if(INPUTS::key_pressed(INPUTS::Key::KEY_S))
			{
				printf("KEY S \n");
			}
			if(INPUTS::joy_key_pressed(INPUTS::JoyKey::KEY_ARROW_DOWN))
			{
				printf("ARROW DOWN \n");
			}
			if(INPUTS::joy_key_pressed(INPUTS::JoyKey::KEY_ARROW_UP))
			{
				printf("ARROW UP \n");
			}
			if(INPUTS::joy_key_pressed(INPUTS::JoyKey::KEY_ARROW_LEFT))
			{
				printf("ARROW LEFT \n");
			}
			if(INPUTS::joy_key_pressed(INPUTS::JoyKey::KEY_ARROW_RIGHT))
			{
				printf("ARROW RIGHT \n");
			}
			INPUTS::update_keys();
			//printf("pitch %f : yaw %f \n",camera.pitch,camera.yaw);
			accumulator -= dt;
		}
#if !VR
		glClearColor(1.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_CULL_FACE);  
		printf("REEE\n");
#endif
		//void render(RenderData* renderables,int numRenderables,
		//	MeshData* meshes,ShaderManager* shaders ,
		//		const SystemUniforms* uniforms,const Camera* camera,uint* textureIds);
		render(&renderData,1,&meshes,&shaders,&sysUniforms,
				&camera,textures.textureIds,eyes,vrVaos,vrMaterial);
		ImGui::Render();
		int display_w, display_h;
		//glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		//glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		//glClearColor(1.f, 0.f, 0.f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//glfwMakeContextCurrent(window);
		glCheckError();

		glfwSwapBuffers(window);
		PROFILER::end_timer(PROFILER::TimerID::Iteration,&timers);
		//printf("RELOADING \n");fflush(stdout);
		hotload_shaders(&shaders,&workingMemory);
	}
	PROFILER::show_timers(&timers);
	glfwTerminate();
	return 0;
}
void render(RenderData* renderables,int numRenderables,
		MeshData* meshes,ShaderManager* shaders ,
		const SystemUniforms* uniforms,const Camera* camera,uint* textureIds,
		FrameTexture* frameTextures,uint* eyeVaos,Material vrProgram)
{
#if !VR
	glBindBuffer(GL_UNIFORM_BUFFER,uniforms->matrixUniformBufferObject);
	glBufferSubData(GL_UNIFORM_BUFFER,0,sizeof(MATH::mat4) * 2, (void*)camera);
	glBindBuffer(GL_UNIFORM_BUFFER,0);
#endif

	struct lightutil{
		MATH::vec4 dir;
		MATH::vec4 ambient;
		MATH::vec4 diffuse;
		MATH::vec4 specular;
	} light;

	light.dir = MATH::vec4(1.f, -1.0f, 0.f,1.f);
	light.ambient = MATH::vec4(0.3f, 0.3f, 0.3f,1.f);
	light.diffuse = MATH::vec4(0.8f, 0.8f, 0.8f,1.f);
	light.specular = MATH::vec4( 0.5f, 0.5f, 0.5f,1.f);
	glBindBuffer(GL_UNIFORM_BUFFER,uniforms->globalLightBufferObject);
	glCheckError();
	glBufferSubData(GL_UNIFORM_BUFFER,0,sizeof(MATH::vec4) * 4, &light);
	glCheckError();
	glBindBuffer(GL_UNIFORM_BUFFER,0);

	glCheckError();
	//FIRST PASS
	//glBindFramebuffer(GL_FRAMEBUFFER,frameBuffer);
#if VR
	//glClearColor(1.f, 0.f, 0.f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE); 

	for(FrameTexture* eye = frameTextures; eye < frameTextures + 2; eye++)
	{
		set_and_clear_frameTexture(*eye);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_CULL_FACE); 
		glCheckError();

		glBindBuffer(GL_UNIFORM_BUFFER,uniforms->matrixUniformBufferObject);
		glBufferSubData(GL_UNIFORM_BUFFER,0,sizeof(MATH::mat4) * 2, (void*)camera);
		glBindBuffer(GL_UNIFORM_BUFFER,0);


#endif
		for(RenderData* i = renderables; i < renderables + numRenderables; i++)
		{
			ShaderProgram* prog = &shaders->shaderPrograms[i->material.shaderProgram];
			uint glID = shaders->shaderProgramIds[i->material.shaderProgram];
			glUseProgram(glID);

#if 1
			MATH::quaternion q(MATH::vec3(
						i->oriTemp.x * MATH::deg_to_rad,
						i->oriTemp.y * MATH::deg_to_rad,
						i->oriTemp.z * MATH::deg_to_rad
						));
			//	printf("%.3f :: %.3f :: %.3f :: %.3f\n",q.i,q.j,q.k,q.scalar);
			//printf("lenght %.4f \n",MATH::lenght(q));
			MATH::mat4 model(q);//i->orientation);
			MATH::translate(&model,i->position);
			MATH::scale_mat4(&model,i->scale);
#endif
			//glUniformMatrix4fv(prog->modelUniformPosition, 1, GL_FALSE, (GLfloat*)model.mat);
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
					case UniformType::MODEL:
						{
							glUniformMatrix4fv(info->location, 1, GL_FALSE, (GLfloat*)&model);//.mat);
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

#if VR
	}
	//set_and_clear_frameTexture();
	//SECOND PASS
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE); 
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	glUseProgram(shaders->shaderProgramIds[vrProgram.shaderProgram]);
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glCheckError();
	ShaderProgram* prog = &shaders->shaderPrograms[vrProgram.shaderProgram];
	glBindVertexArray(eyeVaos[0]);
	UniformInfo* info = &prog->uniforms[0];
	glActiveTexture(GL_TEXTURE0 + info->glTexLocation);
	glBindTexture(GL_TEXTURE_2D,frameTextures[0].texture);
	glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

	glBindVertexArray(eyeVaos[1]);
	glBindTexture(GL_TEXTURE_2D,frameTextures[1].texture);
	glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
	glCheckError();

#endif
}

