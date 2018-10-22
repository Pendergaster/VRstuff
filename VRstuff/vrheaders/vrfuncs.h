#ifndef VR_FUNCS
#define VR_FUNCS
#include "dxgi.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_GL.h"
#include <Utils.h>

static ovrSession session;
static ovrHmdDesc desc;
static long long frameIndex = 0;
static ovrSizei windowSize ; //= { hmdDesc.Resolution.w / 2, hmdDesc.Resolution.h / 2 };
static  ovrMirrorTexture mirrorTexture = nullptr; // ???
struct OculusTextureBuffer;
static GLuint texIdMirror = 0;
static GLuint mirrorFBO = 0;
OculusTextureBuffer * eyeRenderTexture[2] = { nullptr, nullptr };


struct OculusTextureBuffer
{
    ovrTextureSwapChain ColorTextureChain;
    ovrTextureSwapChain DepthTextureChain;
    GLuint              fboId;
    OVR::Sizei	            texSize;

    OculusTextureBuffer(OVR::Sizei size, int sampleCount) :
        ColorTextureChain(nullptr),
        DepthTextureChain(nullptr),
        fboId(0),
        texSize(0, 0)
    {
        assert(sampleCount <= 1); // The code doesn't currently handle MSAA textures.

        texSize = size;

        // This texture isn't necessarily going to be a rendertarget, but it usually is.
        assert(session); // No HMD? A little odd.

        ovrTextureSwapChainDesc descTex = {};
        descTex.Type = ovrTexture_2D;
        descTex.ArraySize = 1;
        descTex.Width = size.w;
        descTex.Height = size.h;
        descTex.MipLevels = 1;
        descTex.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
        descTex.SampleCount = sampleCount;
        descTex.StaticImage = ovrFalse;

        {
            ovrResult result = ovr_CreateTextureSwapChainGL(session, &descTex, &ColorTextureChain);

            int length = 0;
            ovr_GetTextureSwapChainLength(session, ColorTextureChain, &length);

            if(OVR_SUCCESS(result))
            {
                for (int i = 0; i < length; ++i)
                {
                    GLuint chainTexId;
                    ovr_GetTextureSwapChainBufferGL(session, ColorTextureChain, i, &chainTexId);
                    glBindTexture(GL_TEXTURE_2D, chainTexId);

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                }
            }
        }

        descTex.Format = OVR_FORMAT_D32_FLOAT;

        {
            ovrResult result = ovr_CreateTextureSwapChainGL(session, &descTex, &DepthTextureChain);

            int length = 0;
            ovr_GetTextureSwapChainLength(session, DepthTextureChain, &length);

            if (OVR_SUCCESS(result))
            {
              for (int i = 0; i < length; ++i)
              {
                GLuint chainTexId;
                ovr_GetTextureSwapChainBufferGL(session, DepthTextureChain, i, &chainTexId);
                glBindTexture(GL_TEXTURE_2D, chainTexId);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
              }
            }
        }

        glGenFramebuffers(1, &fboId);
    }

    ~OculusTextureBuffer()
    {
        if (ColorTextureChain)
        {
            ovr_DestroyTextureSwapChain(session, ColorTextureChain);
            ColorTextureChain = nullptr;
        }
        if (DepthTextureChain)
        {
            ovr_DestroyTextureSwapChain(session, DepthTextureChain);
            DepthTextureChain = nullptr;
        }
        if (fboId)
        {
            glDeleteFramebuffers(1, &fboId);
            fboId = 0;
        }
    }

    OVR::Sizei GetSize() const
    {
        return texSize;
    }

    void SetAndClearRenderSurface()
    {
        glClearColor(1.f, 0.f, 0.f, 1.0f);
        GLuint curColorTexId;
        GLuint curDepthTexId;
        {
            int curIndex;
            ovr_GetTextureSwapChainCurrentIndex(session, ColorTextureChain, &curIndex);
            ovr_GetTextureSwapChainBufferGL(session, ColorTextureChain, curIndex, &curColorTexId);
        }
        {
          int curIndex;
          ovr_GetTextureSwapChainCurrentIndex(session, DepthTextureChain, &curIndex);
          ovr_GetTextureSwapChainBufferGL(session, DepthTextureChain, curIndex, &curDepthTexId);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, fboId);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curColorTexId, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, curDepthTexId, 0);

        glViewport(0, 0, texSize.w, texSize.h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_FRAMEBUFFER_SRGB);
    }

    void UnsetRenderSurface()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fboId);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
    }

    void Commit()
    {
        ovr_CommitTextureSwapChain(session, ColorTextureChain);
        ovr_CommitTextureSwapChain(session, DepthTextureChain);
    }
};
//#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#if 0
static ovrGraphicsLuid GetDefaultAdapterLuid()
{
    ovrGraphicsLuid luid = ovrGraphicsLuid();

#if defined(_WIN32)
    IDXGIFactory* factory = nullptr;

    if (SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&factory))))
    {
        IDXGIAdapter* adapter = nullptr;

        if (SUCCEEDED(factory->EnumAdapters(0, &adapter)))
        {
            DXGI_ADAPTER_DESC desc;

            adapter->GetDesc(&desc);
            memcpy(&luid, &desc.AdapterLuid, sizeof(luid));
            adapter->Release();
        }

        factory->Release();
    }
#endif

    return luid;
}
#endif
//#undef SUCCEEDED
static int Compare(const ovrGraphicsLuid& lhs, const ovrGraphicsLuid& rhs)
{
    return memcmp(&lhs, &rhs, sizeof(ovrGraphicsLuid));
}



void init_vr_platform()
{
	bool vrSucc = false; //Application();
	ovrGraphicsLuid luid;
	//resolutions, and info about device
	
	do{
        ovrInitParams initParams = { ovrInit_RequestVersion | ovrInit_FocusAware, OVR_MINOR_VERSION, NULL, 0, 0 };
        ovrResult result = ovr_Initialize(&initParams);
		//ovrResult result = ovr_Initialize(nullptr);
		if (OVR_FAILURE(result))
			break;

		result = ovr_Create(&session, &luid);
		if (OVR_FAILURE(result))
		{
			ovr_Shutdown();
            ABORT_MESSAGE("OVR INIT FAIL \n");
			break;
		}

       // if (Compare(luid, GetDefaultAdapterLuid())) // If luid that the Rift is on is not the default adapter LUID...
        //{
         //   ABORT_MESSAGE("FLUID FAIL FAIL \n");
            //VALIDATE(false, "OpenGL supports only the default graphics adapter.");
        //}
		//float frustomHorizontalFOV = session->CameraFrustumHFovInRadians;

		desc = ovr_GetHmdDesc(session);
		ovrSizei resolution = desc.Resolution;

		ovr_SetTrackingOriginType(session, ovrTrackingOrigin_FloorLevel);
			vrSucc = true;
	}while(false);
	
	windowSize = { desc.Resolution.w / 2, desc.Resolution.h / 2 };
	#if 1
	 for (int eye = 0; eye < 2; ++eye)
    {
        ovrSizei idealTextureSize = ovr_GetFovTextureSize(session, ovrEyeType(eye), desc.DefaultEyeFov[eye], 1);
        eyeRenderTexture[eye] = new OculusTextureBuffer( idealTextureSize, 1);

        if (!eyeRenderTexture[eye]->ColorTextureChain || !eyeRenderTexture[eye]->DepthTextureChain)
        {
			ABORT_MESSAGE("failed to create vr frame buffers \n");
            //if (retryCreate) goto Done;
            //VALIDATE(false, "Failed to create texture.");
        }
    }
	#endif
	// taa on jotain???
	ovrMirrorTextureDesc desc_mirror;
    memset(&desc_mirror, 0, sizeof(desc_mirror));
    desc_mirror.Width = windowSize.w;
    desc_mirror.Height = windowSize.h;
    desc_mirror.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

    // Create mirror texture and an FBO used to copy mirror texture to back buffer
    ovrResult result = ovr_CreateMirrorTextureWithOptionsGL(session, &desc_mirror, &mirrorTexture);
    if (!OVR_SUCCESS(result))
    {
		ABORT_MESSAGE("FAILED TO DO MIRROR \n");
        //if (retryCreate) goto Done;
        //VALIDATE(false, "Failed to create mirror texture.");
    }
	ovr_GetMirrorTextureBufferGL(session, mirrorTexture, &texIdMirror);

    glGenFramebuffers(1, &mirrorFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texIdMirror, 0);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	ovr_SetTrackingOriginType(session, ovrTrackingOrigin_FloorLevel);
	LOG("VR INITIALIZED !\n");
}

void dispose_vr_platform()
{
	ovr_Destroy(session);
	ovr_Shutdown();
}

void update_vr_state()
{
#if 0
			ovrSessionStatus sessionStatus;
			ovr_GetSessionStatus(session, &sessionStatus);
			if (sessionStatus.ShouldQuit){
				printf("ovr exit \n");
				ABORT_MESSAGE("OVR fail");
			}
			if (sessionStatus.ShouldRecenter){
					ovr_RecenterTrackingOrigin(session);		
			}
            if(!sessionStatus.IsVisible){
                LOG("NOT VISIBLE \n");
                return;
            }
			
			//ovrResult result = ovr_WaitToBeginFrame(session, frameIndex);
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
#if 0
				Matrix4f eyeOneOrientation(EyeRenderPose[0].Orientation);
				Matrix4f eyeTwoOrientation(EyeRenderPose[1].Orientation);
				Vector3f finalUpOne = eyeOneOrientation.Transform(Vector3f(0, 1, 0));
				Vector3f finalUpTwo = eyeTwoOrientation.Transform(Vector3f(0, 1, 0));
				Vector3f finalForwardOne = eyeOneOrientation.Transform(Vector3f(0, 0, -1));
				Vector3f finalForwardTwo = eyeTwoOrientation.Transform(Vector3f(0, 0, -1));
				Vector3f shiftedEyePos = rollPitchYaw.Transform(EyeRenderPose[eye].Position);
#endif	
			}
#endif
	
}

void render_vr(const RenderCommands& commands)
{


    ovrSessionStatus sessionStatus;
    ovr_GetSessionStatus(session, &sessionStatus);
    if (sessionStatus.ShouldQuit) {
        printf("ovr exit \n");
        ABORT_MESSAGE("OVR fail");
    }
    if (sessionStatus.ShouldRecenter) {
        ovr_RecenterTrackingOrigin(session);
    }
    if (!sessionStatus.IsVisible) {
        LOG("NOT VISIBLE \n");
        return;
    }

    //ovrResult result = ovr_WaitToBeginFrame(session, frameIndex);
#if 0
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
            eyeRenderDesc[1].HmdToEyePose };
        double sensorSampleTime;
        //sensorSampleTime is fed into the layer later
        ovr_GetEyePoses(session, frameIndex, ovrTrue, HmdToEyePose, EyeRenderPose, &sensorSampleTime);

        printf("%.3f, %.3f --  %.3f \n",
            EyeRenderPose[0].Position.x, EyeRenderPose[0].Position.y, EyeRenderPose[0].Position.z);
#if 0
        Matrix4f eyeOneOrientation(EyeRenderPose[0].Orientation);
        Matrix4f eyeTwoOrientation(EyeRenderPose[1].Orientation);
        Vector3f finalUpOne = eyeOneOrientation.Transform(Vector3f(0, 1, 0));
        Vector3f finalUpTwo = eyeTwoOrientation.Transform(Vector3f(0, 1, 0));
        Vector3f finalForwardOne = eyeOneOrientation.Transform(Vector3f(0, 0, -1));
        Vector3f finalForwardTwo = eyeTwoOrientation.Transform(Vector3f(0, 0, -1));
        Vector3f shiftedEyePos = rollPitchYaw.Transform(EyeRenderPose[eye].Position);
#endif	
    }
#endif









    /**********************************************************/


    //for (int eye = 0; eye < 2; eye++)
    //{
    //    eyeRenderTexture[eye]->SetAndClearRenderSurface();
    //    eyeRenderTexture[eye]->UnsetRenderSurface();

    //    // Commit changes to the textures so they get picked up frame
    //    eyeRenderTexture[eye]->Commit();
    //}

    ovrEyeRenderDesc eyeRenderDesc[2];
    eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, desc.DefaultEyeFov[0]);
    eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, desc.DefaultEyeFov[1]);

    // Get eye poses, feeding in correct IPD offset
    ovrPosef EyeRenderPose[2];
    ovrPosef HmdToEyePose[2] = { eyeRenderDesc[0].HmdToEyePose,
                                 eyeRenderDesc[1].HmdToEyePose };
    double sensorSampleTime;    // sensorSampleTime is fed into the layer later

    ovr_GetEyePoses(session, frameIndex, ovrTrue, HmdToEyePose, 
        EyeRenderPose, &sensorSampleTime);

    ovrTimewarpProjectionDesc posTimewarpProjectionDesc = {};


   

    for (int eye = 0; eye < 2; ++eye)
    {
        // Switch to eye render target
        eyeRenderTexture[eye]->SetAndClearRenderSurface();

        // Get view and projection matrices
        OVR::Matrix4f rollPitchYaw = OVR::Matrix4f::RotationY(0);
        OVR::Matrix4f finalRollPitchYaw = rollPitchYaw * OVR::Matrix4f(EyeRenderPose[eye].Orientation);
        OVR::Vector3f finalUp = finalRollPitchYaw.Transform(OVR::Vector3f(0, 1, 0));
        OVR::Vector3f finalForward = finalRollPitchYaw.Transform(OVR::Vector3f(0, 0, -1));
        OVR::Vector3f shiftedEyePos =/* Pos2 +*/ rollPitchYaw.Transform(EyeRenderPose[eye].Position);

        OVR::Matrix4f viewProj[2];
        viewProj[0] = OVR::Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + finalForward, finalUp);
        viewProj[1] = ovrMatrix4f_Projection(desc.DefaultEyeFov[eye], 0.2f, 1000.0f, ovrProjection_None);
        posTimewarpProjectionDesc = ovrTimewarpProjectionDesc_FromProjection(viewProj[1], ovrProjection_None);


        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_CULL_FACE);
        glCheckError();
        glBindBuffer(GL_UNIFORM_BUFFER, commands.uniforms->matrixUniformBufferObject);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MATH::mat4) * 2, (void*)&viewProj);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);


        for (int i = 0; i < commands.numRenderables; i++)
        {
            int currentIndex = i; //commands.renderIndexes[i];
            RenderData* currentRenderData = &commands.renderables[currentIndex];
            Material* currentMaterial = &commands.materials[currentRenderData->materialID];

            ShaderProgram* prog = &commands.shaders->shaderPrograms[currentMaterial->shaderProgram];
            uint glID = commands.shaders->shaderProgramIds[currentMaterial->shaderProgram];
            glUseProgram(glID);


            MATH::quaternion q(MATH::vec3(
                currentRenderData->oriTemp.x * MATH::deg_to_rad,
                currentRenderData->oriTemp.y * MATH::deg_to_rad,
                currentRenderData->oriTemp.z * MATH::deg_to_rad
            ));
            //	printf("%.3f :: %.3f :: %.3f :: %.3f\n",q.i,q.j,q.k,q.scalar);
            //printf("lenght %.4f \n",MATH::lenght(q));
            MATH::mat4 model(q);//i->orientation);
            MATH::translate(&model, currentRenderData->position);
            MATH::scale_mat4(&model, currentRenderData->scale);

            //glUniformMatrix4fv(prog->modelUniformPosition, 1, GL_FALSE, (GLfloat*)model.mat);
            glCheckError();
            for (uint i2 = 0; i2 < currentMaterial->numUniforms; i2++)
            {
                Uniform* uniToSet = &commands.shaders->uniforms[currentMaterial->uniformIndex + i2];
                UniformInfo* info = &prog->uniforms[i2];

                //for(int texPlace = 0; texPlace < prog->)
                //	prog->uniforms[i2].type
                switch (info->type)
                {
                    case UniformType::VEC4:
                    {
                        glUniform4f(info->location, uniToSet->_vec4.x,
                            uniToSet->_vec4.y, uniToSet->_vec4.z, uniToSet->_vec4.w);
                    }break;
                    case UniformType::MAT4:
                    {
                        glUniformMatrix4fv(info->location, 1, GL_FALSE,
                            (GLfloat*)uniToSet->_mat4.mat);
                    }break;
                    case UniformType::FLOATTYPE:
                    {
                        glUniform1f(info->location, uniToSet->_float);
                    }break;
                    case UniformType::INTTYPE:
                    {
                        glUniform1i(info->location, uniToSet->_int);
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
                glDrawElements(GL_TRIANGLES, currentMeshInfo->numIndexes, GL_UNSIGNED_INT, 0);
                glCheckError();
            }




        // Render world
        //roomScene->Render(view, proj);

        // Avoids an error when calling SetAndClearRenderSurface during next iteration.
        // Without this, during the next while loop iteration SetAndClearRenderSurface
        // would bind a framebuffer with an invalid COLOR_ATTACHMENT0 because the texture ID
        // associated with COLOR_ATTACHMENT0 had been unlocked by calling wglDXUnlockObjectsNV.
        eyeRenderTexture[eye]->UnsetRenderSurface();

        // Commit changes to the textures so they get picked up frame
        eyeRenderTexture[eye]->Commit();
    }





    ovrLayerEyeFovDepth ld = {};
    ld.Header.Type = ovrLayerType_EyeFovDepth;
    ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.

    ld.ProjectionDesc = posTimewarpProjectionDesc;

    for (int eye = 0; eye < 2; ++eye)
    {
        ld.ColorTexture[eye] = eyeRenderTexture[eye]->ColorTextureChain;
        ld.DepthTexture[eye] = eyeRenderTexture[eye]->DepthTextureChain;
        ld.Viewport[eye] = OVR::Recti(eyeRenderTexture[eye]->GetSize());
        ld.Fov[eye] = desc.DefaultEyeFov[eye];
        ld.RenderPose[eye] = EyeRenderPose[eye];
        ld.SensorSampleTime = sensorSampleTime;
    }



	ovrLayerHeader* layers = &ld.Header;
    ovrResult result = ovr_SubmitFrame(session, frameIndex, nullptr, &layers, 1);

	if (!OVR_SUCCESS(result))
	{
		ABORT_MESSAGE("RENDERING FAIL  VR \n");//goto Done;
	}

    frameIndex++;
}

#endif //VR_FUNCS