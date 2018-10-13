#define _CRT_SECURE_NO_WARNINGS
//#define _ITERATOR_DEBUG_LEVEL 0
#define STB_IMAGE_IMPLEMENTATION
#define VR 0
#if VR
#include <dxgi.h>
#endif
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
#include <smallDLLloader.h>
#include <gameDefs.h>
#include <ShaderDefs.h>
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

void key_callback(GLFWwindow* , int key, int , int action, int )
{
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


//TODO textuuri set BOE BOE 
//TODO camera 
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
#if 0
void render(RenderData* renderables,int numRenderables,
		MeshData* meshes,ShaderManager* shaders ,
		const SystemUniforms* uniforms,const Camera* camera,uint* textureIds,
		FrameTexture* frameTextures,uint* eyeVaos,Material vrProgramMaterial);
#endif

GLFWwindow* window = NULL;
#if VR
//static OGL Platform;
//OVR::GLEContext         GLEContext;
// Include the OculusVR SDK
#include <OVR_CAPI.h>
bool Application()
{
	ovrResult result = ovr_Initialize(nullptr);
	if (OVR_FAILURE(result))
		return false;

	ovrSession session;
	ovrGraphicsLuid luid;
	result = ovr_Create(&session, &luid);
	if (OVR_FAILURE(result))
	{
		ovr_Shutdown();
		return false;
	}

	ovrHmdDesc desc = ovr_GetHmdDesc(session);
	ovrSizei resolution = desc.Resolution;

	ovr_Destroy(session);
	ovr_Shutdown();
	return true;
}
#endif
struct DLLHotloadHandle
{
	FILESYS::FileHandle fileHandle;
	char*				dllname;
	DLLHandle			dllHandle;
};
bool hotload_dll(DLLHotloadHandle* dll)
{
	FILESYS::FileHandle tempHandle;
	if(!FILESYS::get_filehandle(dll->dllname,&tempHandle)){
		return false;
	}
	if(!FILESYS::compare_file_times(tempHandle,dll->fileHandle))
	{
		LOG("Reloading dll %s \n",dll->dllname);
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
			printf("dll loaded successfully \n");
			return true;
		}
	}
	return false;
}
#if 0
void render(RenderData* renderables,int numRenderables,
		Material* materials,int numMaterials,
		MeshData* meshes,ShaderManager* shaders ,
		const SystemUniforms* uniforms,const Camera* camera,uint* textureIds,
		FrameTexture* frameTextures,uint* eyeVaos,Material vrProgram)
{

#endif
	struct RenderCommands
	{
		RenderData*		renderables;
		int*			renderIndexes = NULL;
		int				numRenderables = 0;
		Material*		materials = NULL;
		Camera*			camera = NULL;
		ShaderManager*	shaders = NULL;
		uint*			textureIds = NULL;
		FrameTexture*	frameTextures = NULL;
		uint*           eyeVaos = NULL;
		SystemUniforms* uniforms = NULL;
		Material		vrProgram;
		MeshData*		meshes = NULL;
	};

	void render(RenderCommands commands);
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
		ImGuiContext* imguiContext = ImGui::GetCurrentContext();
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		const char* glsl_version = "#version 130";
		ImGui_ImplOpenGL3_Init(glsl_version);

		// Setup style
		ImGui::StyleColorsDark();
		INPUTS::Input inputs;
		init_inputs(&inputs);

		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		const double dt = 1.0 / 60.0;
		double currentTime = glfwGetTime();
		double accumulator = 0.0;

#if 0
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

#endif
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

#if VR
		bool vrSucc = false; //Application();
		ovrSession session;
		ovrGraphicsLuid luid;
		//resolutions, and info about device
		ovrHmdDesc desc;
		defer{ ovr_Destroy(session); };
		defer{ ovr_Shutdown(); };
		do{
			ovrResult result = ovr_Initialize(nullptr);
			if (OVR_FAILURE(result))
				break;


			result = ovr_Create(&session, &luid);
			if (OVR_FAILURE(result))
			{
				ovr_Shutdown();
				break;
			}

			//float frustomHorizontalFOV = session->CameraFrustumHFovInRadians;




			desc = ovr_GetHmdDesc(session);
			ovrSizei resolution = desc.Resolution;


#if 0
			ovrTrackingState ts = ovr_GetTrackingState(session, ovr_GetTimeInSeconds(), ovrTrue);
			if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked)) 
			{
				ovrPosef pose = ts.HeadPose.ThePose;
				...
			}

#endif
			ovr_SetTrackingOriginType(session, ovrTrackingOrigin_FloorLevel);
			vrSucc = true;
		}while(false);

		FrameTexture eyes[2] =
		{ create_new_frameTexture(desc.Resolution.w / 2,desc.Resolution.h / 2,GL_COLOR_ATTACHMENT0,FrameBufferAttacment::Color | FrameBufferAttacment::Depth)
			,create_new_frameTexture(desc.Resolution.w / 2,desc.Resolution.h / 2,GL_COLOR_ATTACHMENT0,FrameBufferAttacment::Color | FrameBufferAttacment::Depth) };

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		if(vrSucc){ printf("JEI \n");}
		if(!vrSucc){ printf("NEI \n");}


#endif

		DLLHotloadHandle gameDLL;
#if defined (_WIN32)
		gameDLL.dllname = (char*)"game/DebugBin/game.dll";
#elif __linux__
		gameDLL.dllname = (char*)"./game/DebugBin/game.lib";
#endif
		func_ptr init_game;
		func_ptr update_game;
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
		}

		GameHook hook;
		hook.shaders = &shaders;
		hook.meshes = &meshes;
		hook.textures = &textures;
		hook.imguiContext = imguiContext;
		CONTAINER::init_memory_block(&hook.gameMemory,GAME_MEMORY_SIZE);
		printf("context set\n");
		init_game(&hook);
		PROFILER::end_timer(PROFILER::TimerID::Start,&timers);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::EndFrame();
#if VR
		long long frameIndex = 0;
		//ovrHmdDesc hmdDesc = ovr_GetHmdDesc(session);
#endif
		while (!glfwWindowShouldClose(window))
		{
#if VR
			ovrSessionStatus sessionStatus;
			ovr_GetSessionStatus(session, &sessionStatus);
			if (sessionStatus.ShouldQuit){
				printf("ovr exit \n");
				break;
			}
			ovrResult result = ovr_WaitToBeginFrame(session, frameIndex);
			ovrTrackingState ts = ovr_GetTrackingState(session, ovr_GetTimeInSeconds(), ovrTrue);
			if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked)) 
			{
				ovrPosef pose = ts.HeadPose.ThePose;
				//...


				ovrEyeRenderDesc eyeRenderDesc[2];
				eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, desc.DefaultEyeFov[0]);
				eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, desc.DefaultEyeFov[1]);

				ovrPosef EyeRenderPose[2];
				ovrPosef HmdToEyePose[2] = { eyeRenderDesc[0].HmdToEyePose,
					eyeRenderDesc[1].HmdToEyePose};
				double sensorSampleTime;    
				//sensorSampleTime is fed into the layer later
				ovr_GetEyePoses(session, frameIndex, ovrTrue, HmdToEyePose, EyeRenderPose, &sensorSampleTime);

				printf("%.3f, %.3f --  %.3f \n",
						EyeRenderPose[0].Position.x, EyeRenderPose[0].Position.y, EyeRenderPose[0].Position.z);

				Matrix4f eyeOneOrientation(EyeRenderPose[0].Orientation);
				Matrix4f eyeTwoOrientation(EyeRenderPose[1].Orientation);
				Vector3f finalUpOne = eyeOneOrientation.Transform(Vector3f(0, 1, 0));
				Vector3f finalUpTwo = eyeTwoOrientation.Transform(Vector3f(0, 1, 0));
				Vector3f finalForwardOne = eyeOneOrientation.Transform(Vector3f(0, 0, -1));
				Vector3f finalForwardTwo = eyeTwoOrientation.Transform(Vector3f(0, 0, -1));
				Vector3f shiftedEyePos = rollPitchYaw.Transform(EyeRenderPose[eye].Position);
			}

#endif
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
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				//printf("new frame\n");fflush(stdout);
				update_game(&hook);
				INPUTS::update_keys();
				//printf("pitch %f : yaw %f \n",camera.pitch,camera.yaw);
				accumulator -= dt;
				{
					//static float f = 0.0f;
					static int counter = 0;

					ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

					ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
					static bool show = false;
					static bool show2 = false;
					ImGui::Checkbox("Demo Window", &show);      // Edit bools storing our window open/close state
					ImGui::Checkbox("Another Window", &show2);


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

				//printf("end frame\n");
				ImGui::EndFrame();
			}
#if !VR
			glClearColor(1.f, 0.f, 0.f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_MULTISAMPLE);
			glEnable(GL_CULL_FACE);  
			//printf("REEE\n");
#endif
			//void render(RenderData* renderables,int numRenderables,
			//	MeshData* meshes,ShaderManager* shaders ,
			//		const SystemUniforms* uniforms,const Camera* camera,uint* textureIds);
			RenderCommands rend;
			rend.camera = &camera;
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
			rend.vrProgram = vrMaterial;
			rend.renderIndexes = hook.renderIndexes;
			rend.eyeVaos = vrVaos;

			//render(&renderData,1,&meshes,&shaders,&sysUniforms,
			//		&camera,textures.textureIds,eyes,vrVaos,vrMaterial);
			render(rend);
			//printf("renderings \n");fflush(stdout);;
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

			}
#endif
		}
		PROFILER::show_timers(&timers);
		glfwTerminate();
		return 0;
	}
	void render(RenderCommands commands)
	{
#if !VR
		glBindBuffer(GL_UNIFORM_BUFFER,commands.
				uniforms->matrixUniformBufferObject);
		glBufferSubData(GL_UNIFORM_BUFFER,
				0,sizeof(MATH::mat4) * 2, (void*)commands.camera);
		glBindBuffer(GL_UNIFORM_BUFFER,0);
#if 0	
		glBindBuffer(GL_UNIFORM_BUFFER,commands.
				uniforms->cameraBlockBufferObject);
		MATH::vec4 tempPos(
				commands.camera->position.x,
				commands.camera->position.y,
				commands.camera->position.z,
				1
				);

		glBufferSubData(GL_UNIFORM_BUFFER,
				0,sizeof(MATH::mat4), (void*)&tempPos);
		glBindBuffer(GL_UNIFORM_BUFFER,0);
#endif

#endif

		struct lightutil{
			MATH::vec4 dir;
			MATH::vec4 ambient;
			MATH::vec4 diffuse;
			MATH::vec4 specular;
		} light;
		glCheckError();
		light.dir = MATH::vec4(1.f, -1.0f, 0.f,1.f);
		light.ambient = MATH::vec4(0.3f, 0.3f, 0.3f,1.f);
		light.diffuse = MATH::vec4(0.8f, 0.8f, 0.8f,1.f);
		light.specular = MATH::vec4( 0.5f, 0.5f, 0.5f,1.f);
		glBindBuffer(GL_UNIFORM_BUFFER,commands.uniforms->globalLightBufferObject);
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

		for(FrameTexture* eye = commands.frameTextures; eye < commands.frameTextures + 2; eye++)
		{
			set_and_clear_frameTexture(*eye);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_MULTISAMPLE);
			glEnable(GL_CULL_FACE); 
			glCheckError();

			glBindBuffer(GL_UNIFORM_BUFFER,commands.uniforms->matrixUniformBufferObject);
			glBufferSubData(GL_UNIFORM_BUFFER,0,sizeof(MATH::mat4) * 2, (void*)commands.camera);
			glBindBuffer(GL_UNIFORM_BUFFER,0);


#endif
			for(int i = 0; i < commands.numRenderables; i++)
			{
				int currentIndex = commands.renderIndexes[i];
				RenderData* currentRenderData = &commands.renderables[currentIndex];
				Material* currentMaterial = &commands.materials[currentRenderData->materialID];

				ShaderProgram* prog = &commands.shaders->shaderPrograms[currentMaterial->shaderProgram];
				uint glID = commands.shaders->shaderProgramIds[currentMaterial->shaderProgram];
				glUseProgram(glID);

#if 1
				MATH::quaternion q(MATH::vec3(
							currentRenderData->oriTemp.x * MATH::deg_to_rad,
							currentRenderData->oriTemp.y * MATH::deg_to_rad,
							currentRenderData->oriTemp.z * MATH::deg_to_rad
							));
				//	printf("%.3f :: %.3f :: %.3f :: %.3f\n",q.i,q.j,q.k,q.scalar);
				//printf("lenght %.4f \n",MATH::lenght(q));
				MATH::mat4 model(q);//i->orientation);
				MATH::translate(&model,currentRenderData->position);
				MATH::scale_mat4(&model,currentRenderData->scale);
#endif
				//glUniformMatrix4fv(prog->modelUniformPosition, 1, GL_FALSE, (GLfloat*)model.mat);
				glCheckError();
				for(uint i2 = 0; i2 < currentMaterial->numUniforms;i2++)
				{
					Uniform* uniToSet = &commands.shaders->uniforms[currentMaterial->uniformIndex + i2];
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
								glBindTexture(GL_TEXTURE_2D, commands.textureIds[uniToSet->_textureCacheId]);
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
				Mesh* currentMesh = &commands.meshes->meshArray[currentRenderData->meshID];
				MeshInfo* currentMeshInfo = &commands.meshes->meshInfos[currentRenderData->meshID];
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
		glUseProgram(commands.shaders->shaderProgramIds[commands.vrProgram.shaderProgram]);
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glCheckError();
		ShaderProgram* prog = &commands.shaders->shaderPrograms[commands.vrProgram.shaderProgram];
		glBindVertexArray(commands.eyeVaos[0]);
		UniformInfo* info = &prog->uniforms[0];
		glActiveTexture(GL_TEXTURE0 + info->glTexLocation);
		glBindTexture(GL_TEXTURE_2D,commands.frameTextures[0].texture);
		glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

		glBindVertexArray(commands.eyeVaos[1]);
		glBindTexture(GL_TEXTURE_2D,commands.frameTextures[1].texture);
		glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
		glCheckError();

#endif
	}

