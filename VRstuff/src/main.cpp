#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
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
#define STATIC_MEM_SIZE 1000000
#define WORKING_MEM_SIZE 10000000
#include "renderer.h"
#if VR
#include <vrfuncs.h>
#endif

static void error_callback(int e, const char *d)
{
	printf("Error %d: %s\n", e, d);
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
				LOG("file locked %s",dll->dllname);
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
int main()
{
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
	ModelCache models;
	TextureData textures;
	init_textures_from_metadata(&textures,&staticMemory);
	fill_model_cache(&models,&workingMemory,&staticMemory);

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
	Renderer renderer;
	renderer.globalUniforms = &sysUniforms;
	init_renderer(&renderer,&shaders,&textures,&workingMemory);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//uint canvasVao = 0;

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
	hook.models = &models;
	hook.textures = &textures;
	hook.globalLight.dir = MATH::vec4(0.5f, -1.0f, 0.f,1.f);
	hook.globalLight.ambient = MATH::vec4(0.2f, 0.2f, 0.2f,1.f);
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

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

	glEnable(GL_MULTISAMPLE);
	float runTime = (float)glfwGetTime();
	//FrameTexture depthMap = create_depth_texture(2048,2048);

	while (!glfwWindowShouldClose(window))
	{
#if VR
		update_vr_state();
#endif
		glfwPollEvents();
		PROFILER::start_timer(PROFILER::TimerID::Iteration,&timers);
		double newTime = glfwGetTime();
		runTime = (float)newTime;
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

			ImGui::EndFrame();
		}
		glCheckError();
		renderer.light = hook.globalLight;
		renderer.runtime = runTime;
		renderer.view = hook.viewMatrix;
		renderer.projection = hook.projectionMatrix;
		renderer.numRenderables = hook.numRenderables;
		renderer.renderData = hook.renderables;
		renderer.materials = hook.materials;
		renderer.animations = hook.animations;

		render_pass(&renderer,&models,&shaders,&textures,&workingMemory);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glfwGetFramebufferSize(window, &display_w, &display_h);
		//glViewport(0, 0, display_w, display_h);
		glCheckError();

		glfwSwapBuffers(window);

	}

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
	LOG("trying to dispose game");
	dispose_game(&hook);
	PROFILER::show_timers(&timers);
	glfwTerminate();
	printf("Bye \n");
	return 0;
}

//#define VR 1
