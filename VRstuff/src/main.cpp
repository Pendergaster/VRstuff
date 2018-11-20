#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include <algorithm>
#include<limits.h>
#include <assert.h>
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
#include <imgui/imgui_demo.cpp>
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui_impl_glfw.h"
#include "imgui_impl_glfw.cpp"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_opengl3.cpp"
#endif

//#include <file_system.h>
#include<JsonToken.h>
#include <MathUtil.h>
#include<ModelData.h>
#include<shader_utils.h>
#include<Utils.h>
#include<Containers.h>
#include <smallDLLloader.h>
#include <gameDefs.h>
#include <ShaderDefs.h>
#include <sharedinputs.h>
#include "input.h"
// data for reloading and opengl state
#include <texturedefs.h>
#include "textures.h"
#include "meshes.h"
#include "shaders.h"
#include "glerrorcheck.h"
#include "timer.h"
#include "SystemUniforms.h"
#include "input.h"
#if 1
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>
#endif
#define STATIC_MEM_SIZE 100000
#define WORKING_MEM_SIZE 100000
struct FrameTexture
{
	uint texture;
	uint buffer;
	uint textureWidth;
	uint textureHeight;
	int	 attachments;
};
struct RenderCommands
{
	RenderData*			renderables;
	//int*				renderIndexes = NULL;
	int					numRenderables = 0;
	Material*			materials = NULL;
	MATH::mat4			view;
	MATH::mat4			projection;
	glm::mat4			shadowMatrixes[NUM_CASCADES];
	float				clipPositions[NUM_CASCADES];

	ShaderManager*		shaders = NULL;
	uint*				textureIds = NULL;
	SystemUniforms*		uniforms = NULL;
	MeshData*			meshes = NULL;
	FrameTexture		offscreen;
	//uint				shadowMap;
	FrameTexture*		shadowMaps;
	struct GlobalLight*	globalLight;
};

static uint skyvao = 0;
static Material skymaterial;

#if VR
#include <vrfuncs.h>
#endif

static void error_callback(int e, const char *d)
{
	printf("Error %d: %s\n", e, d);
}

enum FrameBufferAttacment : int
{
	None = 1 << 0,
	Color = 1 << 1,
	Depth = 1 << 2,
	Multisample = 1 << 3
};

static inline FrameTexture create_depth_texture(uint width,uint height)
{
	FrameTexture ret;
	ret.attachments = FrameBufferAttacment::Depth;
	ret.textureHeight = height;
	ret.textureWidth = width;
	glGenFramebuffers(1, &ret.buffer);  
	glBindFramebuffer(GL_FRAMEBUFFER,ret.buffer);

	glGenTextures(1, &ret.texture);
	glBindTexture(GL_TEXTURE_2D, ret.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
			width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, ret.buffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ret.texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);  
	return ret;
}
static inline FrameTexture create_new_frameTexture(uint width,uint height,GLenum attachment,int type)
{

	FrameTexture ret;
	ret.textureWidth = width;
	ret.textureHeight = height;
	glGenFramebuffers(1,&ret.buffer);
	glBindFramebuffer(GL_FRAMEBUFFER,ret.buffer);
	glGenTextures(1,&ret.texture);
	GLenum texturetype = 0;
	texturetype = BIT_CHECK(type,FrameBufferAttacment::Multisample) ? 
		GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D; 
	glBindTexture(texturetype,ret.texture);
	if(BIT_CHECK(type,FrameBufferAttacment::Multisample))
	{
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,NULL);
	}
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glBindTexture(texturetype,0);
	glFramebufferTexture2D(GL_FRAMEBUFFER,attachment,texturetype,ret.texture,0);
	if(type != FrameBufferAttacment::Depth && BIT_CHECK(type,FrameBufferAttacment::Depth))
	{
		uint rbo = 0;
		glGenRenderbuffers(1,&rbo);
		glBindRenderbuffer(GL_RENDERBUFFER,rbo);
		//TODO tähän vr dimensiot
		if(BIT_CHECK(type,FrameBufferAttacment::Multisample))
		{
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4,
					GL_DEPTH24_STENCIL8, width, height);  
		}
		else
		{

			glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,
					ret.textureWidth,ret.textureHeight);
		}
		glBindRenderbuffer(GL_RENDERBUFFER,0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,
				GL_RENDERBUFFER,rbo);
	}
	if(!(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)){
		ABORT_MESSAGE("FAILED TO SET FRAMEBUFFER \n");
	}
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	ret.attachments = type;
	glCheckError();
	return ret;
}
static inline void set_and_clear_frameTexture(const FrameTexture& frameTex)
{
	glCheckError();
	glBindFramebuffer(GL_FRAMEBUFFER,frameTex.buffer);
	glViewport(0, 0, frameTex.textureWidth, frameTex.textureHeight);
	glClearColor(0.f,0.f,0.f,1.f);
	if(BIT_CHECK(frameTex.attachments,FrameBufferAttacment::Depth) && BIT_CHECK(frameTex.attachments,FrameBufferAttacment::Color)  ) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	else if ( FrameBufferAttacment::Depth == frameTex.attachments ) {
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	else if ( FrameBufferAttacment::Color == frameTex.attachments) {
		glClear(GL_COLOR_BUFFER_BIT);
	}
	else
	{
		ABORT_MESSAGE("Error with frametexturetype !!");
	}
	glCheckError();
}

static void inline blit_frameTexture(FrameTexture from,FrameTexture to)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, from.buffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, to.buffer);
	glBlitFramebuffer(0, 0, from.textureWidth, from.textureHeight, 0, 0,
			to.textureWidth, to.textureHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST); 

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glCheckError();
}

static void inline blit_frameTexture(FrameTexture from,uint to)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, from.buffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, to);
	glBlitFramebuffer(0, 0, from.textureWidth, from.textureHeight, 0, 0,
			SCREENWIDHT, SCREENHEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST); 
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glCheckError();
}


GLFWwindow* window = NULL;

struct DLLHotloadHandle
{
	FILESYS::FileHandle fileHandle;
	char*				dllname;
	DLLHandle			dllHandle = 0;
};
bool hotload_dll(DLLHotloadHandle* dll)
{
	FILESYS::FileHandle tempHandle;
	if(!FILESYS::get_filehandle(dll->dllname,&tempHandle)){
		return false;
	}
	if(!FILESYS::compare_file_times(tempHandle,dll->fileHandle))
	{
		LOG("Reloading dll %s ",dll->dllname);
#if defined (_WIN32)
		if(FILESYS::does_file_exist(".lock"))
#elif __linux__
			if(FILESYS::does_file_exist("./.lock"))
#endif
			{
				printf("file locked %s \n",dll->dllname);
				return false;
			}
		dll->fileHandle = tempHandle;
		if(FILESYS::does_file_exist(dll->dllname))
		{
			UnloadDLL(&dll->dllHandle);
			if(!load_DLL(&dll->dllHandle,dll->dllname))
			{
				ABORT_MESSAGE("failed to load game \n");
			}
			LOG("dll loaded successfully ");
			return true;
		}
	}
	return false;
}
void render(const RenderCommands& commands);
void render_depth(const RenderCommands& commands,Material shadowMat);
int main()
{
	assert(sizeof(MATH::mat4)==sizeof(glm::mat4));
	PROFILER::TimerCache timers;
	PROFILER::init_timers(&timers);
	PROFILER::start_timer(PROFILER::TimerID::Start,&timers);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwSwapInterval(0);
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

	LOG("Game inited "); fflush(stdout);
	glCheckError();

	SystemUniforms sysUniforms;
	glCheckError();
	init_systemuniforms(&sysUniforms,shaders.shaderPrograms,
			shaders.shaderProgramIds,shaders.numShaderPrograms);



	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGuiContext* imguiContext = ImGui::GetCurrentContext();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	const char* glsl_version = "#version 130";
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Setup style
	ImGui::StyleColorsDark();

	const double dt = 1.0 / 60.0;
	double currentTime = glfwGetTime();
	double accumulator = 0.0;
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	uint canvasVao = 0;
	Material postCanvas;
	{
		float canvasVerts[] = 
		{
			// positions          // colors           // texture coords
			1.f,  1.f,   1.0f, 1.0f,   // top right
			1.f, -1.f,   1.0f, 0.0f,   // bottom right
			-1.f, -1.f,  0.0f, 0.0f,   // bottom left
			-1.f,  1.f,  0.0f, 1.0f    // top left 
		};

		unsigned int canvasInds[] =
		{  // note that we start from 0!
			3, 1, 0,				// first triangle
			3, 2, 1					// second triangle
		}; 

		uint buffers[2];
		glGenBuffers(2,buffers);
		glGenVertexArrays(1,&canvasVao);
		glBindVertexArray(canvasVao);

		glBindBuffer(GL_ARRAY_BUFFER,buffers[0]);
		glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,
				sizeof(float) * 4,(void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,
				sizeof(float) * 4,(void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glBufferData(GL_ARRAY_BUFFER,sizeof(canvasVerts),NULL,GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(canvasVerts),canvasVerts);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,buffers[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(canvasInds), nullptr, GL_STATIC_DRAW);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(canvasInds), canvasInds);
		glBindVertexArray(0);
		postCanvas = create_new_material(&shaders,"PostPro");
	}

#if VR
	init_vr_platform();
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	defer{dispose_vr_platform();};
#endif

	DLLHotloadHandle gameDLL;
#if defined (_WIN32)
	gameDLL.dllname = (char*)"game/DebugBin/game.dll";
#elif __linux__
	gameDLL.dllname = (char*)"./game/DebugBin/game.lib";
#endif
	func_ptr init_game;
	func_ptr update_game;
	func_ptr on_game_reload;
	func_ptr dispose_game;
	{
#if defined (_WIN32)
		if(!load_DLL(&gameDLL.dllHandle,gameDLL.dllname))
#elif __linux__
			if(!load_DLL(&gameDLL.dllHandle,gameDLL.dllname))
#endif
			{
				ABORT_MESSAGE("failed to load game \n");
			}
		init_game = load_DLL_function(gameDLL.dllHandle,"init_game");
		if(!init_game){
			ABORT_MESSAGE("Failed to load init game \n");
		}
		update_game = load_DLL_function(gameDLL.dllHandle,"update_game");
		if(!update_game){
			ABORT_MESSAGE("Failed to load update game \n");
		}
		on_game_reload = load_DLL_function(gameDLL.dllHandle,"on_game_reload");
		if(!on_game_reload){
			ABORT_MESSAGE("Failed to load on_game_reload game \n");
		}
		dispose_game = load_DLL_function(gameDLL.dllHandle,"dispose_game");
		if(!dispose_game){
			ABORT_MESSAGE("Failed to load dispose_game game \n");
		}
		if(!FILESYS::get_filehandle(gameDLL.dllname,&gameDLL.fileHandle))
		{
			ABORT_MESSAGE("SOMETHING WENT WRONG \n");
		}
	}

	GameHook hook;
	init_inputs(&hook.inputs);
	hook.shaders = &shaders;
	hook.meshes = &meshes;
	hook.textures = &textures;
	hook.globalLight.dir = MATH::vec4(0.5f, -1.0f, 0.f,1.f);
	hook.globalLight.ambient = MATH::vec4(0.05f, 0.05f, 0.05f,1.f);
	hook.globalLight.diffuse = MATH::vec4(0.8f, 0.8f, 0.8f,1.f);
	hook.globalLight.specular = MATH::vec4( 0.5f, 0.5f, 0.5f,1.f);

	hook.imguiContext = imguiContext;
	CONTAINER::init_memory_block(&hook.gameMemory,GAME_MEMORY_SIZE);
	CONTAINER::init_memory_block(&hook.workingMemory,GAME_WORKING_MEMORY);
	init_game(&hook);
	PROFILER::end_timer(PROFILER::TimerID::Start,&timers);
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::EndFrame();
	skyvao = 0;
#if 1
	{
		float skyboxVertices[] = {
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f,  1.0f
		};

		skymaterial = create_new_material(&shaders,"SkyProg");

		set_material_texture(&shaders,&skymaterial,0,get_texture(textures,"sky_box"));

		glGenVertexArrays(1,&skyvao);
		uint skyvbo = 0;
		glGenBuffers(1,&skyvbo);
		glBindVertexArray(skyvao);
		glBindBuffer(GL_ARRAY_BUFFER,skyvao);
		glBindBuffer(GL_ARRAY_BUFFER,skyvbo);

		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, 
				GL_STATIC_DRAW);

		glCheckError();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
		glEnableVertexAttribArray(0);
		glCheckError();

		glBindVertexArray(0);

		glCheckError();
	}
#endif
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

	glEnable(GL_MULTISAMPLE);
	FrameTexture offscreen = create_new_frameTexture(SCREENWIDHT,
			SCREENHEIGHT, GL_COLOR_ATTACHMENT0, 
			FrameBufferAttacment::Color | FrameBufferAttacment::Depth 
			| FrameBufferAttacment::Multisample  );
	FrameTexture postProcessCanvas = create_new_frameTexture(SCREENWIDHT,
			SCREENHEIGHT, GL_COLOR_ATTACHMENT0, 
			FrameBufferAttacment::Color );

	FrameTexture cascades[NUM_CASCADES];
	for(uint i = 0; i < NUM_CASCADES;i++)
	{
		cascades[i] = create_depth_texture(2048,2048);
	}

	//FrameTexture depthMap = create_depth_texture(2048,2048);
	Material shadowMaterial = create_new_material(&shaders,"ShadowProg");


	while (!glfwWindowShouldClose(window))
	{
#if VR
		update_vr_state();
#endif
		glfwPollEvents();
		PROFILER::start_timer(PROFILER::TimerID::Iteration,&timers);
		double newTime = glfwGetTime();
		double frameTime = newTime - currentTime;
		currentTime = newTime;
		accumulator += frameTime;


		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		{
			break;
		}
		while(accumulator >= dt)
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
#if 0
			//ImGui::ShowDemoWindow(&show_demo_window);
#endif

			static bool show_console = false;
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("Windows"))
				{
					ImGui::MenuItem("Console", NULL, &show_console);
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
			if(show_console)
			{
				static ExampleAppConsole console;
				console.Draw("Example: Console", &show_console);

			}
			hook.runTime = (float)glfwGetTime();
			update_game(&hook);
			update_keys();
			accumulator -= dt;
			{
#if 1
				ImGui::Begin("Hello, world!");                          
#define NUM_FRUSTRUM_CORNERS 8

#if 0
				print_vec4((*(MATH::vec4*)&hook.viewMatrix.mat[0][0]),"view");
				print_vec4((*(MATH::vec4*)&hook.viewMatrix.mat[1][0]),"view");
				print_vec4((*(MATH::vec4*)&hook.viewMatrix.mat[2][0]),"view");
				print_vec4((*(MATH::vec4*)&hook.viewMatrix.mat[3][0]),"view");
#endif
#if 0
				ImGui::Text("x1  %.3f ", x1);
				ImGui::Text("x2  %.3f ", x2);
				ImGui::Text("cam pos %.3f %.3f %.3f ", _camPos.x,_camPos.y,_camPos.z);
				ImGui::Text("cam look %.3f %.3f %.3f ", _camLook.x,_camLook.y,_camLook.z);
				ImGui::Text("cascade info f %.3f ba %.3f r %.3f l %.3f t %.3f bo %.3f " ,
						orthoInfo->f,orthoInfo->n,orthoInfo->r,orthoInfo->l,orthoInfo->t,orthoInfo->b);
#endif
#if 0
				print_vec(xFRU,"FRU");
				print_vec(xFRD,"FRD");
				print_vec(xFLU,"FLU");
				print_vec(xFLD,"FLD");

				print_vec(xNRU,"NRU");
				print_vec(xNRD,"NRD");
				print_vec(xNLU,"NLU");
				print_vec(xNLD,"NLD");
#endif

				ImGui::End();


#if 0
				ImGui::Begin("Scene Window");
				//get the mouse position

				ImGui::Image(&postProcessCanvas.texture, ImVec2(100,100));
				ImVec2 pos = ImGui::GetCursorScreenPos();

				//pass the texture of the FBO
				//window.getRenderTexture() is the texture of the FBO
				//the next parameter is the upper left corner for the uvs to be applied at
				//the third parameter is the lower right corner
				//the last two parameters are the UVs
				//they have to be flipped (normally they would be (0,0);(1,1) 
				ImGui::GetWindowDrawList()->AddImage(
						&cascades[0].texture, 
						ImVec2(ImGui::GetCursorScreenPos()),
						ImVec2(ImGui::GetCursorScreenPos().x + 100 / 2, 
							ImGui::GetCursorScreenPos().y + 100/2), ImVec2(0, 1), ImVec2(1, 0));

				//we are done working with this window


				ImGui::End();

#endif

#endif
			}


			ImGui::EndFrame();
		}

		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_CULL_FACE);  
		glCullFace(GL_BACK);  

		RenderCommands rend;
		//rend.projection = hook.projectionMatrix;
		//rend.view = hook.viewMatrix;
		rend.offscreen = offscreen;
		rend.globalLight = &hook.globalLight;
#if VR
		rend.frameTextures = eyes;
#endif
		rend.meshes = &meshes;
		rend.renderables = hook.renderables;
		rend.numRenderables = hook.numRenderables;
		rend.shaders = &shaders;
		rend.textureIds = textures.textureIds;
		rend.materials = hook.materials;
		rend.uniforms = &sysUniforms;
#if 0
		set_and_clear_frameTexture(depthMap);
		// 1. first render to depth map
		//ConfigureShaderAndMatrices();
		MATH::mat4 shadowOrtho;
		MATH::mat4 shadowLookat;
		glm::mat4 inv = glm::inverse(*(glm::mat4*)&hook.viewMatrix);
		glm::vec3 _camPos(inv[3][0], inv[3][1], inv[3][2]);
		glm::vec3 _camLook(inv[2][0], inv[2][1], inv[2][2]);
		_camLook *= -1.f;

		//printf("%.3f  %.3f  %.3f \n",_camPos.x,_camPos.y,_camPos.z);

		float near_plane = 0.5f, far_plane = 40.f;
		glm::vec3 lightDir = (*(glm::vec3*)&hook.globalLight.dir);
		MATH::normalize((MATH::vec3*)&lightDir);
		MATH::normalize((MATH::vec3*)&_camLook);
		MATH::scale((MATH::vec3*)&_camLook , 10000.f/ 2.f);
		MATH::scale((MATH::vec3*)&lightDir , far_plane / 2.f);

		glm::mat4 lightView = glm::lookAt(
				(_camPos + _camLook) - lightDir,//glm::vec3(-3.0f, 8.0f, -1.0f),
				_camPos + _camLook,
				//glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
		shadowLookat = *(MATH::mat4*)&lightView;
		glm::mat4 lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f,
				near_plane, far_plane);
		shadowOrtho =  *(MATH::mat4*)&lightProjection;
		//MATH::orthomat(&shadowOrtho,-10.f, 10.f,-10.f,10.f,1.f, 7.5f);
		//MATH::create_lookat_mat4(&shadowLookat,
		//	MATH::vec3(-2.0f, 4.0f, -1.0f),
		//MATH::vec3(0.0f, 0.0f, 0.0f),
		//	MATH::vec3(0.0f, 1.0f, 0.0f));

		//MATH::mat4 lightSpaceMatrix = shadowOrtho * shadowLookat; 
		rend.projection = shadowOrtho;
		rend.view = shadowLookat;
		glCullFace(GL_FRONT);
		render_depth(rend,shadowMaterial);
		glCullFace(GL_BACK);
		//RenderScene();
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// 2. then render scene as normal with shadow mapping (using depth map)
		//glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//ConfigureShaderAndMatrices();
		//glBindTexture(GL_TEXTURE_2D, depthMap);
		//RenderScene();
#else
		float x1 = tanf((MATH::deg_to_rad * 90.f) / 2.f );
		glm::mat4 inv = glm::inverse(*(glm::mat4*)&hook.viewMatrix);
#define print_vec(vec,name) ImGui::Text(name " %.3f %.3f %.3f ", vec.x,vec.y,vec.z)
#define print_vec4(vec,name) ImGui::Text(name " %.3f %.3f %.3f %.3f", vec.x,vec.y,vec.z,vec.w)

		glm::vec3 _camPos(inv[3][0], inv[3][1], inv[3][2]);
		glm::vec3 _camLook(inv[2][0], inv[2][1], inv[2][2]);
		glm::vec3 lightDir = (*(glm::vec3*)&hook.globalLight.dir);
		glm::mat4 lightView = glm::lookAt(glm::vec3(0,0,0),lightDir,glm::vec3(0,1,0));
		//lookAt (detail::tvec3< T > const &eye, detail::tvec3< T > const &center, detail::tvec3< T > const &up)
		float cascadeEnds[4 +1] = {
			0.1f, 10.f, 40.f,60.f , 100.f
		};
		struct OrthoInfo{

			float r,l,b,t,n,f;
		} orthoInfo[NUM_CASCADES];
		for (uint i = 0;i < NUM_CASCADES; i ++)
		{
			float xn = cascadeEnds[i] * x1;
			float xf = cascadeEnds[i + 1] * x1;
			float yn = cascadeEnds[i] * x1;
			float yf = cascadeEnds[i + 1] * x1;
			glm::vec4 corners[NUM_FRUSTRUM_CORNERS] = 
			{
				glm::vec4 (xn,yn,cascadeEnds[i] ,1),
				glm::vec4 (-xn,yn,cascadeEnds[i] ,1),
				glm::vec4 (xn,-yn,cascadeEnds[i] ,1),
				glm::vec4 (-xn,-yn,cascadeEnds[i] ,1),

				glm::vec4 (xf,yf,cascadeEnds[i + 1] ,1),
				glm::vec4 (-xf,yf,cascadeEnds[i + 1] ,1),
				glm::vec4 (xf,-yf,cascadeEnds[i + 1] ,1),
				glm::vec4 (-xf,-yf,cascadeEnds[i + 1] ,1)
			};

			glm::vec4 cornersL[NUM_FRUSTRUM_CORNERS];
			float minX = std::numeric_limits<float>::max();
			float maxX = std::numeric_limits<float>::min();
			float minY = std::numeric_limits<float>::max();
			float maxY = std::numeric_limits<float>::min();
			float minZ = std::numeric_limits<float>::max();
			float maxZ = std::numeric_limits<float>::min();


			for(int j = 0; j < NUM_FRUSTRUM_CORNERS; j++)
			{
				glm::vec4 vW = inv * corners[j];
				//TODO TÄHÄN VALON TRANSFORM!!!!!	
				cornersL[j] = lightView * vW;
				//print_vec(cornersL[j],"vW");

				minX = std::min(minX,cornersL[j].x);
				maxX = std::max(maxX,cornersL[j].x);
				minY = std::min(minY,cornersL[j].y);
				maxY = std::max(maxY,cornersL[j].y);
				minZ = std::min(minZ,cornersL[j].z);
				maxZ = std::max(maxZ,cornersL[j].z);
			}

			orthoInfo[i].r = maxX;
			orthoInfo[i].l = minX;
			orthoInfo[i].b = minY;
			orthoInfo[i].t = maxY;
			orthoInfo[i].n = minZ;
			orthoInfo[i].f = maxZ;
		}

		// lenght cascade
		rend.clipPositions[0] = (*(glm::mat4*)&hook.projectionMatrix * 
				glm::vec4(0,0,cascadeEnds[1],1)).z;
		rend.clipPositions[1] = (*(glm::mat4*)&hook.projectionMatrix * 
				glm::vec4(0,0,cascadeEnds[2],1)).z;
		rend.clipPositions[2] = (*(glm::mat4*)&hook.projectionMatrix *
				glm::vec4(0,0,cascadeEnds[3],1)).z;
		rend.clipPositions[3] = (*(glm::mat4*)&hook.projectionMatrix *
				glm::vec4(0,0,cascadeEnds[4],1)).z;

		struct bindableMatrix{
			glm::mat4 view,ortho;
		} bind;
		bind.view = lightView;
		ShaderProgram* prog = 
			&shaders.shaderPrograms[shadowMaterial.shaderProgram];
		uint glID = shaders.shaderProgramIds[shadowMaterial.shaderProgram];
		glUseProgram(glID);
		ASSERT_MESSAGE(prog->uniforms[0].type == UniformType::MODEL,"SHADOW PROG INVALID UNIFORM");
		uint modelPos =  prog->uniforms[0].location;

		//glm::mat4 shadowMatrixes[NUM_CASCADES]; 
		//render every cascade
		for(uint i = 0; i < NUM_CASCADES; i++)
		{
			set_and_clear_frameTexture(cascades[i]);
			bind.ortho = glm::ortho(
					orthoInfo[i].l,orthoInfo[i].r,
					orthoInfo[i].b,orthoInfo[i].t,
					orthoInfo[i].n,orthoInfo[i].f);
			rend.shadowMatrixes[i] = bind.ortho * bind.view;
			glBindBuffer(GL_UNIFORM_BUFFER,
					sysUniforms.matrixUniformBufferObject);
			glBufferSubData(GL_UNIFORM_BUFFER,
					0,sizeof(bindableMatrix), (void*)&bind);
			glBindBuffer(GL_UNIFORM_BUFFER,0);
			for(int j = 0; j < hook.numRenderables; j++)
			{
				RenderData* currentRenderData = 
					&hook.renderables[j];

				MATH::mat4 model(currentRenderData->orientation);
				MATH::translate(&model,currentRenderData->position);
				MATH::scale_mat4(&model,currentRenderData->scale);
				glCheckError();

				glUniformMatrix4fv(modelPos, 1, 
						GL_FALSE, (GLfloat*)&model);//.mat);
				glCheckError();
				//set mesh
				Mesh* currentMesh = 
					&meshes.meshArray[currentRenderData->meshID];
				MeshInfo* currentMeshInfo = 
					&meshes.meshInfos[currentRenderData->meshID];
				glBindVertexArray(currentMesh->vao);
				glDrawElements(GL_TRIANGLES,currentMeshInfo->numIndexes,
						GL_UNSIGNED_INT,0);
				glCheckError();
			}
		}
		float x = orthoInfo[0].r - orthoInfo[0].l;
		float y = orthoInfo[0].t - orthoInfo[0].b;
		float z = orthoInfo[0].f - orthoInfo[0].n;
		x = x < 0 ? -x : x;
		y = y < 0 ? -y : y;
		z = z < 0 ? -z : z;
		printf(" volume %.3f \n", x * y * z);

		printf("%.3f %.3f %.3f %.3f %.3f %.3f \n",orthoInfo->n,
				orthoInfo->f, orthoInfo->r,orthoInfo->l  , orthoInfo->b,orthoInfo->t);


#endif
#if 1
		rend.view = hook.viewMatrix;
		rend.projection = hook.projectionMatrix;
		rend.shadowMaps = cascades;
		//rend.shadowMatrixes = shadowMatrixes;
		set_and_clear_frameTexture(offscreen);
		//glm::mat4 lightSpaceMatrix = lightProjection * lightView;
		//rend.shadowMatrix =*(MATH::mat4*)&lightSpaceMatrix;// shadowOrtho * shadowLookat;
		//rend.shadowMap = depthMap.texture;
		render(rend);
		//int display_w, display_h;
		//glfwGetFramebufferSize(window, &display_w, &display_h);
		//glViewport(0, 0, display_w, display_h);
		glCheckError();
#endif
#if VR
		render_vr(rend);
#endif
		glCheckError();
#if 1
		blit_frameTexture(offscreen,postProcessCanvas);
		//blit_frameTexture(postProcessCanvas,0);
		glBindFramebuffer(GL_FRAMEBUFFER,0);


		glDisable(GL_DEPTH_TEST);
		glUseProgram( shaders.shaderProgramIds[postCanvas.shaderProgram]);
		SHADER::set_vec2_name( shaders.shaderProgramIds[postCanvas.shaderProgram],"pos"
				,MATH::vec2(0,0));
		SHADER::set_vec2_name( shaders.shaderProgramIds[postCanvas.shaderProgram],"scale"
				,MATH::vec2(1,1));
		glBindVertexArray(canvasVao);
		glCheckError();
		glActiveTexture(GL_TEXTURE0);
		glCheckError();
		glBindTexture(GL_TEXTURE_2D,
				//depthMap.texture);
			postProcessCanvas.texture);
		glCheckError();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 


		SHADER::set_vec2_name( shaders.shaderProgramIds[postCanvas.shaderProgram],"pos"
				,MATH::vec2(-0.75,-0.75));
		SHADER::set_vec2_name( shaders.shaderProgramIds[postCanvas.shaderProgram],"scale"
				,MATH::vec2(0.25,0.25));
		glBindVertexArray(canvasVao);
		glCheckError();
		glActiveTexture(GL_TEXTURE0);
		glCheckError();
		glBindTexture(GL_TEXTURE_2D,
				cascades->texture);
		//postProcessCanvas.texture);
		glCheckError();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 

		SHADER::set_vec2_name( shaders.shaderProgramIds[postCanvas.shaderProgram],"pos"
				,MATH::vec2(0.75,-0.75));
		SHADER::set_vec2_name( shaders.shaderProgramIds[postCanvas.shaderProgram],"scale"
				,MATH::vec2(0.25,0.25));
		glBindVertexArray(canvasVao);
		glCheckError();
		glActiveTexture(GL_TEXTURE0);
		glCheckError();
		glBindTexture(GL_TEXTURE_2D,
				cascades[2].texture);
		//postProcessCanvas.texture);
		glCheckError();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 



		glEnable(GL_DEPTH_TEST);

#else
		blit_frameTexture(offscreen,0);
#endif





		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glfwGetFramebufferSize(window, &display_w, &display_h);
		//glViewport(0, 0, display_w, display_h);
		glCheckError();

		glfwSwapBuffers(window);
		PROFILER::end_timer(PROFILER::TimerID::Iteration,&timers);
#if 1
		hotload_shaders(&shaders,&workingMemory);
		if(hotload_dll(&gameDLL))
		{
			init_game = load_DLL_function(gameDLL.dllHandle,"init_game");
			if(!init_game){
				ABORT_MESSAGE("Failed to load init game \n");
			}
			update_game = load_DLL_function(gameDLL.dllHandle,"update_game");
			if(!update_game){
				ABORT_MESSAGE("Failed to load update game \n");
			}
			on_game_reload = load_DLL_function(gameDLL.dllHandle,"on_game_reload");
			if(!on_game_reload){
				ABORT_MESSAGE("Failed to load on_game_reload game \n");
			}
			dispose_game = load_DLL_function(gameDLL.dllHandle,"dispose_game");
			if(!dispose_game){
				ABORT_MESSAGE("Failed to load dispose_game game \n");
			}
			on_game_reload(&hook);
		}
#endif
	}
	printf("trying to dispose game \n");
	dispose_game(&hook);
	PROFILER::show_timers(&timers);
	glfwTerminate();
	printf("Bye \n");
	return 0;
}

struct PointLight
{
	MATH::vec3 pos;
	FrameTexture depthMap;
	//todo vals and stuff
};

void render_depth(const RenderCommands& commands,Material shadowMat)
{
	glBindBuffer(GL_UNIFORM_BUFFER,commands.
			uniforms->matrixUniformBufferObject);
	glBufferSubData(GL_UNIFORM_BUFFER,
			0,sizeof(MATH::mat4) * 2, (void*)&commands.view);
	glBindBuffer(GL_UNIFORM_BUFFER,0);

	glBindBuffer(GL_UNIFORM_BUFFER,commands.
			uniforms->shadowBlockBufferObject);
	glBufferSubData(GL_UNIFORM_BUFFER,
			0,sizeof(MATH::mat4) * 2, (void*)&commands.view);
	glBindBuffer(GL_UNIFORM_BUFFER,0);




	ShaderProgram* prog = &commands.shaders->shaderPrograms[shadowMat.shaderProgram];
	uint glID = commands.shaders->shaderProgramIds[shadowMat.shaderProgram];
	glUseProgram(glID);

	ASSERT_MESSAGE(prog->uniforms[0].type == UniformType::MODEL,"SHADOW PROG INVALID UNIFORM");
	uint modelPos =  prog->uniforms[0].location;


	for(int i = 0; i < commands.numRenderables; i++)
	{
		int currentIndex = i; //commands.renderIndexes[i];
		RenderData* currentRenderData = &commands.renderables[currentIndex];

		MATH::mat4 model(currentRenderData->orientation);
		MATH::translate(&model,currentRenderData->position);
		MATH::scale_mat4(&model,currentRenderData->scale);
		glCheckError();

		glUniformMatrix4fv(modelPos, 1, GL_FALSE, (GLfloat*)&model);//.mat);

		glCheckError();
		//set mesh
		Mesh* currentMesh = 
			&commands.meshes->meshArray[currentRenderData->meshID];
		MeshInfo* currentMeshInfo = 
			&commands.meshes->meshInfos[currentRenderData->meshID];
		glBindVertexArray(currentMesh->vao);
		glDrawElements(GL_TRIANGLES,currentMeshInfo->numIndexes,
				GL_UNSIGNED_INT,0);
		glCheckError();
	}
}
void render(const RenderCommands& commands)
{
	//cameramatrixes
	glBindBuffer(GL_UNIFORM_BUFFER,commands.
			uniforms->matrixUniformBufferObject);
	glBufferSubData(GL_UNIFORM_BUFFER,
			0,sizeOfMatrixBlock,(void*)&commands.view);
	glBindBuffer(GL_UNIFORM_BUFFER,0);
	//global lighting
	glCheckError();
	glBindBuffer(GL_UNIFORM_BUFFER,commands.uniforms->globalLightBufferObject);
	glCheckError();
	glBufferSubData(GL_UNIFORM_BUFFER,0,sizeOfGlobalLightBlock,commands.globalLight);
	glCheckError();
	glBindBuffer(GL_UNIFORM_BUFFER,0);

	//shadowmatrixes and clipspaces
	glCheckError();
	glBindBuffer(GL_UNIFORM_BUFFER,commands.uniforms->shadowBlockBufferObject);
	glCheckError();
	glBufferSubData(GL_UNIFORM_BUFFER,0,sizeOfShadowBlock
			, commands.shadowMatrixes);
	glCheckError();
	glBindBuffer(GL_UNIFORM_BUFFER,0);
	glCheckError();

	for(int i = 0; i < NUM_CASCADES; i++)
	{
		glActiveTexture(GL_TEXTURE0 + SHADOW_MAP_INDEXES + i);
		glBindTexture(GL_TEXTURE_2D,
				commands.shadowMaps[i].texture);
	}

	//FIRST PASS
	for(int i = 0; i < commands.numRenderables; i++)
	{
		int currentIndex = i; //commands.renderIndexes[i];
		RenderData* currentRenderData = &commands.renderables[currentIndex];
		Material* currentMaterial = &commands.materials[currentRenderData->materialID];

		ShaderProgram* prog = &commands.shaders->shaderPrograms[currentMaterial->shaderProgram];
		uint glID = commands.shaders->shaderProgramIds[currentMaterial->shaderProgram];
		glUseProgram(glID);

		MATH::mat4 model(currentRenderData->orientation);
		MATH::translate(&model,currentRenderData->position);
		MATH::scale_mat4(&model,currentRenderData->scale);
		glCheckError();

		//set uniforms
		for(uint i2 = 0; i2 < currentMaterial->numUniforms;i2++)
		{
			Uniform* uniToSet = &commands.shaders->uniforms[currentMaterial->uniformIndex + i2];
			UniformInfo* info = &prog->uniforms[i2];

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
						glBindTexture(GL_TEXTURE_2D,
								commands.textureIds[uniToSet->_textureCacheId]);
					}break;
				case UniformType::MODEL:
					{
						glUniformMatrix4fv(info->location, 1, GL_FALSE, (GLfloat*)&model);//.mat);
					}break;
				case UniformType::SHADOW:
					{
						//glActiveTexture(GL_TEXTURE0 + info->glTexLocation);
						//glBindTexture(GL_TEXTURE_2D,
						//		commands.shadowMap);
					}break;
				case UniformType::INVALID: default:
					{
						ABORT_MESSAGE("INVALID UNIFORM TYPE /n");
					}break;
			}
		}
		glCheckError();
		//set mesh
		Mesh* currentMesh = 
			&commands.meshes->meshArray[currentRenderData->meshID];
		MeshInfo* currentMeshInfo = 
			&commands.meshes->meshInfos[currentRenderData->meshID];
		glBindVertexArray(currentMesh->vao);
		glDrawElements(GL_TRIANGLES,currentMeshInfo->numIndexes,
				GL_UNSIGNED_INT,0);
		glCheckError();
	}
	//render skybox
	glActiveTexture(GL_TEXTURE0);
	glDepthFunc(GL_LEQUAL); 
	uint glID = commands.shaders->shaderProgramIds[skymaterial.shaderProgram];
	glUseProgram(glID);
	glCheckError();
	// ... set view and projection matrix
	MATH::mat4 tempview = commands.view;
	tempview.mat[3][0] = 0;
	tempview.mat[3][1] = 0;
	tempview.mat[3][2] = 0;
	tempview.mat[3][3] = 1;

	glCheckError();
	SHADER::set_mat4_name(glID,"view",tempview.mat);
	SHADER::set_mat4_name(glID,"projection",commands.projection.mat);

	glCheckError();

	glBindVertexArray(skyvao);
	glActiveTexture(GL_TEXTURE0 );

	glCheckError();
	Uniform* uniToSet = &commands.shaders->uniforms[skymaterial.uniformIndex];
	glBindTexture(GL_TEXTURE_CUBE_MAP, commands.textureIds[uniToSet->_textureCacheId]);

	glCheckError();
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glCheckError();
	glDepthFunc(GL_LESS);

	glCheckError();
	glUseProgram(0);
	glBindVertexArray(0);
}
//#define VR 1
