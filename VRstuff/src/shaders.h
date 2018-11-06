#ifndef PAKKI_SHADERS
#define PAKKI_SHADERS
#include <Containers.h>
#include <JsonToken.h>
#include <Utils.h>
#include <stb_image.h>
#include <glad/glad.h>
#include <ModelData.h>
#include <shader_utils.h>
#include <ShaderDefs.h>
#include "SystemUniforms.h"
#include "glerrorcheck.h"

#define MAX_UNIFORMS 1000
#define MAX_SYSTEM_UNIFORMS 100
//TODO inc uni info
static inline bool setup_uniform(UniformInfo* uniform,uint id)
{
	int location = glGetUniformLocation(id,uniform->name);
	if(location == -1) return false;
	uniform->location = location;
	return true;
}
static inline bool setup_uniform_sampler2D(UniformInfo* uniform,uint id,int numTex)
{
	//printf("%s\n",uniform->name);
	int location = glGetUniformLocation(id,uniform->name);
	if(location == -1) return false;
	uniform->glTexLocation = numTex;
	glUniform1i(location, numTex); 
	return true;
}
static bool compile_program(const ShaderProgram& prog,uint* shaderID, char* vertSource, char* fragSource)
{

	glCheckError();
	uint vertSha = 0;
	if(!SHADER::compile_shader(GL_VERTEX_SHADER,vertSource,&vertSha))
	{
		printf("Failed to create shader %s \n",prog.vertexPath);
		return false;
	}
	uint fragSha = 0;
	if(!SHADER::compile_shader(GL_FRAGMENT_SHADER,fragSource,&fragSha))
	{
		printf("Failed to create shader %s \n",prog.fragmentPath);
		return false;
	}
	*shaderID = glCreateProgram();;
	//glAttachShader(*shaderID,vertSha);
	//glAttachShader(*shaderID,fragSha);
	if(!SHADER::link_shader(*shaderID,vertSha,fragSha)){
		return false;
	}
	glCheckError();
	return true;

}
char* modelName = (char*)"model";
static void load_shader_programs(ShaderManager* manager,CONTAINER::MemoryBlock* workingMem,CONTAINER::MemoryBlock* staticMem)
	//	ShaderProgram* programs,uint* shaderIds,int* numIds,
	//	CONTAINER::DynamicArray<char*> names,CONTAINER::StringTable<int> shaderTable,
	//	CONTAINER::MemoryBlock* staticMem,CONTAINER::MemoryBlock* workingMem,JsonToken* token,
	//	CONTAINER::DynamicArray<char*>* uniformNames)
{
	CONTAINER::DynamicArray<char*> names;
	CONTAINER::init_dynamic_array(&names);
	defer {CONTAINER::dispose_dynamic_array(&names);};
	CONTAINER::DynamicArray<char*> uniformNames;
	CONTAINER::init_dynamic_array(&uniformNames);
	defer {CONTAINER::dispose_dynamic_array(&uniformNames);};

	JsonToken token;
	token.ParseFile("importdata/shaderdata.json");
	token.GetKeys(&names);
	CONTAINER::init_table_with_block(&manager->shaderProgramCache,staticMem,names.numobj);

	manager->shaderProgramIds = (uint*)CONTAINER::get_next_memory_block(*staticMem);
	CONTAINER::increase_memory_block(staticMem,names.numobj * sizeof(uint));

	manager->shaderPrograms = (ShaderProgram*)CONTAINER::get_next_memory_block(*staticMem);
	CONTAINER::increase_memory_block(staticMem,MAX_SYSTEM_UNIFORMS * sizeof(ShaderProgram));

	manager->uniforms = (Uniform*)CONTAINER::get_next_memory_block(*staticMem);
	CONTAINER::increase_memory_block(staticMem,MAX_UNIFORMS * sizeof(Uniform));

	manager->uniformInfos = (UniformInfo*)CONTAINER::get_next_memory_block(*staticMem);
	CONTAINER::increase_memory_block(staticMem,MAX_SYSTEM_UNIFORMS * sizeof(UniformInfo));

	CONTAINER::MemoryBlock prevMemState = *workingMem;
	manager->numShaderPrograms = names.numobj;
	for(uint i = 0 ; i < names.numobj; i++)
	{
		char* currentName = names.buffer[i];
		CONTAINER::insert_to_table<int>(
				&manager->shaderProgramCache,currentName,i);
		ShaderProgram currentShaderProg;
		JsonToken* currentToken = token[currentName].GetToken();
		char* vertPath = (*currentToken)["VertexPath"].GetString();
		char* fragPath = (*currentToken)["FragmentPath"].GetString();
		char* vertFile = NULL,* fragFile = NULL;
		if(vertPath && fragPath)
		{
			ASSERT_MESSAGE(vertPath && fragPath,"PATHS NOT DEFINED FOR SHADER :: %s \n",currentName);
			vertFile = SHADER::load_shader(vertPath,workingMem);
			fragFile = SHADER::load_shader(fragPath,workingMem);
			currentShaderProg.type = ShaderType::Normal;

			currentShaderProg.vertexPath = (char*)CONTAINER::get_next_memory_block(*staticMem);
			strcpy(currentShaderProg.vertexPath , vertPath);
			CONTAINER::increase_memory_block_aligned(staticMem,(int)strlen(currentShaderProg.vertexPath) + 1);

			currentShaderProg.fragmentPath = (char*)CONTAINER::get_next_memory_block(*staticMem);
			strcpy(currentShaderProg.fragmentPath , fragPath);
			CONTAINER::increase_memory_block_aligned(staticMem,(int)strlen(currentShaderProg.fragmentPath) + 1);

			FILESYS::get_filehandle(fragPath,
					&currentShaderProg.fragmentFileWriteTime );
			FILESYS::get_filehandle(vertPath,
					&currentShaderProg.vertexFileWriteTime );

		}
		else
		{
			char* combinedPath = (*currentToken)["CombinedPath"].GetString();
			ASSERT_MESSAGE(combinedPath,"PATHS NOT DEFINED FOR SHADER :: %s \n",currentName);
			if(!SHADER::load_frag_and_vert(combinedPath,&vertFile,&fragFile,workingMem)){
				ABORT_MESSAGE("ERROR LOADING SHADER :: %s \n",currentName);
			}
			currentShaderProg.type = ShaderType::Combined;

			currentShaderProg.combiedPath = (char*)CONTAINER::get_next_memory_block(*staticMem);
			strcpy(currentShaderProg.combiedPath , combinedPath);
			CONTAINER::increase_memory_block_aligned(staticMem,(int)strlen(currentShaderProg.combiedPath) + 1);
			FILESYS::get_filehandle(combinedPath,
					&currentShaderProg.combinedFileWriteTime);
		}

		if(!compile_program(currentShaderProg,&manager->shaderProgramIds[i],vertFile,fragFile))
		{
			ABORT_MESSAGE("FAILED TO COMPILE SHADER");
		}

		glUseProgram(manager->shaderProgramIds[i]);
		ValueType t = (*currentToken)["GlobalUniforms"].GetType();
		if(t == ValueType::Jarray)
		{
			int numGlobalUniforms = (*currentToken)["GlobalUniforms"].GetArraySize();
			for(int uniIter = 0; uniIter < numGlobalUniforms;uniIter++)
			{
				char* glUniName = (*currentToken)["GlobalUniforms"][uniIter].GetString();
				ASSERT_MESSAGE(glUniName,"GLOBAL UNIFORM NAME NOT STRING :: %s \n",currentName);
				if(!strcmp(glUniName,"MVP")){
					LOG("DETECTED MVP ");
					//currentShaderProg.globalUniformFlags = 
					BIT_SET(currentShaderProg.globalUniformFlags,GlobalUniforms::MVP);
				}
				else if(!strcmp(glUniName,"GlobalLight")){
					LOG("DETECTED GLOBALIGHT ");
					//currentShaderProg.globalUniformFlags = 
					BIT_SET(currentShaderProg.globalUniformFlags,GlobalUniforms::GlobalLight);
				}
				else if(!strcmp(glUniName,"CameraBlock")){
					LOG("DETECTED CameraBlock ");
					//currentShaderProg.globalUniformFlags = 
					BIT_SET(currentShaderProg.globalUniformFlags,GlobalUniforms::GlobalLight);
				}

				else{
					ABORT_MESSAGE("GLOBALUNIFORM NOT DEFINED %s \n",currentName);
				}
			}
		}
		char* renderGroup = (*currentToken)["RenderGroup"].GetString();
		ASSERT_MESSAGE(renderGroup,"CURRENT SHADER HAS NOT DEFINED RENDERGROUP :: %s \n",
				currentName);
		for(int i3 = 0; i3 < (int)RenderGroup::MaxTypes; i3++)
		{
			if(!strcmp(renderGroup,RENDERGORUP_NAMES[i3]))		
			{
				currentShaderProg.group = (RenderGroup)i3;
				break;
			}
		}
		ASSERT_MESSAGE(currentShaderProg.group != RenderGroup::INVALID,
				"SHADER RENDER GROUP NOT DEFINED CORRECTLY :: %s \n",currentName);
		//if(currentShaderProg.group == RenderGroup::Model)
#if 0
		if(BIT_CHECK(currentShaderProg.globalUniformFlags,GlobalUniforms::MVP))
		{
			LOG("SETTING MVP LOCATION");
			int location = glGetUniformLocation(manager->shaderProgramIds[i],"model");
			ASSERT_MESSAGE(location != GL_INVALID_VALUE,
					"MODEL MATRIX COULD NOT BE FOUND IN :: %s \n",currentName);
			currentShaderProg.modelUniformPosition = location;
		}
#endif

		//currentShaderProg
		JsonToken* uniformToken = (*currentToken)["Uniforms"].GetToken();
		ASSERT_MESSAGE(uniformToken,"SHADER HAS NOT DEFINED UNIFORMS :: %s \n",currentName);
		uniformToken->GetKeys(&uniformNames);
		defer{uniformNames.numobj = 0;};
		currentShaderProg.numUniforms = uniformNames.numobj;
		currentShaderProg.uniforms = &manager->uniformInfos[manager->numSystemUniforms];
		ASSERT_MESSAGE(manager->numSystemUniforms + currentShaderProg.numUniforms < MAX_SYSTEM_UNIFORMS,"SYSTEM UNIFORMS IS EXEEDED");
		manager->numSystemUniforms += currentShaderProg.numUniforms;

		for(uint i2 = 0; i2 < uniformNames.numobj;i2++)
		{
			char* uniformName = uniformNames.buffer[i2];
			char* uniformTypeName = (*uniformToken)[uniformName].GetString();
			ASSERT_MESSAGE(uniformName,"UNIFORM IS NOT STRING TYPE %s in %s \n",uniformName,currentName);
			UniformInfo uniformInfo;
			uniformInfo.name = (char*)CONTAINER::get_next_memory_block(*staticMem);
			CONTAINER::increase_memory_block_aligned(staticMem,(int)strlen(uniformName) + 1);
			strcpy(uniformInfo.name,uniformName);
			uniformInfo.hashedID = CONTAINER::hash(uniformInfo.name);
			//TODO kipase location
			for(int i3 = 0; i3 < UniformType::MaxTypes; i3++)
			{
				if(!strcmp(uniformTypeName,UNIFORM_TYPE_NAMES[i3]))		
				{
					uniformInfo.type = i3;
					break;
				}
			}
			ASSERT_MESSAGE(uniformInfo.type != UniformType::INVALID,
					"UNIFORM %s NOT VALID TYPE IN %s",uniformName,currentName);
			if (uniformInfo.type == UniformType::SAMPLER2D ||
					uniformInfo.type == UniformType::SAMPLERCUBE)
			{
				if(!setup_uniform_sampler2D(&uniformInfo,
							manager->shaderProgramIds[i],currentShaderProg.numTextures++))
				{
					ABORT_MESSAGE("FAILED TO SET UP UNIFORM \n");
				}
			}
			else if(!setup_uniform(&uniformInfo,manager->shaderProgramIds[i]))
			{
				ABORT_MESSAGE("FAILED TO SET UP UNIFORM \n");
			}
			currentShaderProg.uniforms[i2] = uniformInfo;
		}
		if(BIT_CHECK(currentShaderProg.globalUniformFlags,GlobalUniforms::MVP))
		{
			UniformInfo uniformInfo;
			uniformInfo.name = modelName;
			uniformInfo.hashedID = CONTAINER::hash(uniformInfo.name);
			int location = glGetUniformLocation(manager->shaderProgramIds[i],"model");
			ASSERT_MESSAGE(location != GL_INVALID_VALUE,
					"MODEL MATRIX COULD NOT BE FOUND IN :: %s \n",currentName);
			uniformInfo.location = location;
			uniformInfo.type = UniformType::MODEL;
			currentShaderProg.uniforms[currentShaderProg.numUniforms] = uniformInfo;
			++manager->numSystemUniforms; 
			++currentShaderProg.numUniforms;

		}

		manager->shaderPrograms[i] = currentShaderProg;
		*workingMem = prevMemState;
		glUseProgram(0);
	}
}

void hotload_shaders(ShaderManager* manager,CONTAINER::MemoryBlock* workingMem)
{
	CONTAINER::MemoryBlock prevState = *workingMem;
	defer {*workingMem = prevState;};
	int index = 0;
	for(ShaderProgram* i = manager->shaderPrograms; i < 
			manager->shaderPrograms + manager->numShaderPrograms;i++,index++)
	{
		char* vertFile = NULL,* fragFile = NULL;
		if(i->type == ShaderType::Combined)
		{
			FILESYS::FileHandle handle;
			//printf("GETTING HANDLE FOR %s \n",i->combiedPath);fflush(stdout);
			if(!FILESYS::get_filehandle(i->combiedPath,&handle)){
				continue;
			}
			if(!compare_file_times(i->combinedFileWriteTime,handle))
			{
				LOG("HOT RELOADING %s ",i->combiedPath);
				fflush(stdout);
				FILESYS::get_filehandle(i->combiedPath,
						&i->combinedFileWriteTime);

				if(!SHADER::load_frag_and_vert(i->combiedPath,&vertFile,&fragFile,workingMem))
				{
					LOG("Failed to hotload shader :: %s ",i->combiedPath);
					continue;
				}

				uint tempShaderID = 0;
				if(compile_program(*i,&tempShaderID,vertFile,fragFile))
				{
					glUseProgram(tempShaderID);
					defer{ glUseProgram(0); };
					int numTex = 0;
					bool success = true;
					UniformInfo* tempArray = (UniformInfo*)CONTAINER::get_next_memory_block(*workingMem);
					CONTAINER::increase_memory_block(workingMem,sizeof(UniformInfo) * i->numUniforms);
					memcpy(tempArray,i->uniforms,sizeof(UniformInfo) * i->numUniforms);
					//for(UniformInfo* uni = i->uniforms;uni < i->uniforms + i->numUniforms;uni++)
					glCheckError();
					for(UniformInfo* uni = tempArray;uni < tempArray + i->numUniforms;uni++)
					{
						if(uni->type == UniformType::SAMPLER2D)
						{
							if(!setup_uniform_sampler2D(uni,tempShaderID,numTex++)){
								success = false;
								continue;
							}
						} 
						else if(uni->type == UniformType::MODEL)
						{
							int location = glGetUniformLocation(tempShaderID,"model");
							if(location == -1 )
							{
								success = false;
								continue;

							}
							//glGetUniformLocation()
						}
						else
						{
							if(!setup_uniform(uni,tempShaderID)){
								success = false;
								continue;
							}
						}
						glCheckError();
					}
					if(BIT_CHECK(i->globalUniformFlags,GlobalUniforms::MVP) && success)
					{
						if(!set_global_uniform_to_program(tempShaderID,"MatrixBlock",
									MATRIXES_UNIFORM_LOC))
						{
							success = false;
						}
					}
					if(BIT_CHECK(i->globalUniformFlags,GlobalUniforms::GlobalLight) && success)
					{
						//printf("SET LIGHTS");
						//uint lightIndex = glGetUniformBlockIndex(*idIter, "GlobalLight");   
						//glUniformBlockBinding(*idIter, lightIndex, GLOBALLIGHT_UNIFORM_LOC);
						//glCheckError();
						if(!set_global_uniform_to_program(tempShaderID,"GlobalLight",
									GLOBALLIGHT_UNIFORM_LOC))
						{
							success = false;
						}
					}

					if(success)
					{
						LOG("loading succeed ");
						glDeleteProgram(manager->shaderProgramIds[index]);
						manager->shaderProgramIds[index] = tempShaderID;
						memcpy(i->uniforms,tempArray,sizeof(UniformInfo) * i->numUniforms);
					}
					else{
						glDeleteProgram(tempShaderID);
						LOG("loading failed ");
					}

				}

			}

#if 0
			if(!compare_file_times(i->combinedFileWriteTime,handle))
			{
				printf("HOT RELOADING %s \n",i->combiedPath);fflush(stdout);
				if(!SHADER::load_frag_and_vert(i->combiedPath,&vertFile,&fragFile,workingMem))
				{
					LOG("Failed to hotload shader :: %s \n",i->combiedPath);
					continue;
				}
				//TODO FIX COMBIED TYPO
				FILESYS::get_filehandle(i->combiedPath,
						&i->combinedFileWriteTime);
				uint tempShaderID = 0;
				if(compile_program(*i,&tempShaderID,vertFile,fragFile))
				{
					glDeleteProgram(manager->shaderProgramIds[index]);
					manager->shaderProgramIds[index] = tempShaderID;
				}
				//static bool compile_program(ShaderProgram* prog,uint* shaderID,
				//char* vertSource, char* fragSource)

			}
#endif
		}
		else
		{

			FILESYS::FileHandle handleVert;
			FILESYS::FileHandle handleFrag;
			//printf("GETTING HANDLE FOR %s \n",i->vertexPath);fflush(stdout);
			//printf("GETTING HANDLE FOR %s \n",i->fragmentPath);fflush(stdout);
			if(!FILESYS::get_filehandle(i->vertexPath,&handleVert)){
				continue;
				//ABORT_MESSAGE("SHADER MISSING %s /n",i->vertexPath);
			}
			if(!FILESYS::get_filehandle(i->fragmentPath,&handleFrag)){
				continue;
				//ABORT_MESSAGE("SHADER MISSING %s /n",i->fragmentPath);
			}

			if(!compare_file_times(i->vertexFileWriteTime,handleVert) || 
					!compare_file_times(i->fragmentFileWriteTime,handleFrag))
			{
				LOG("HOT RELOADING %s and %s ",i->vertexPath,i->fragmentPath);
				fflush(stdout);

				vertFile = SHADER::load_shader(i->vertexPath,workingMem);
				fragFile = SHADER::load_shader(i->fragmentPath,workingMem);
				FILESYS::get_filehandle(i->vertexPath,
						&i->vertexFileWriteTime);
				FILESYS::get_filehandle(i->fragmentPath,
						&i->fragmentFileWriteTime);

				uint tempShaderID = 0;
				if(compile_program(*i,&tempShaderID,vertFile,fragFile))
				{
					int numTex = 0;
					bool success = true;
					UniformInfo* tempArray = (UniformInfo*)CONTAINER::get_next_memory_block(*workingMem);
					CONTAINER::increase_memory_block(workingMem,sizeof(UniformInfo) * i->numUniforms);
					memcpy(tempArray,i->uniforms,sizeof(UniformInfo) * i->numUniforms);
					//for(UniformInfo* uni = i->uniforms;uni < i->uniforms + i->numUniforms;uni++)
					for(UniformInfo* uni = tempArray;uni < tempArray + i->numUniforms;uni++)
					{
						if(uni->type == UniformType::SAMPLER2D)
						{
							if(!setup_uniform_sampler2D(uni,tempShaderID,numTex++)){
								success = false;
								continue;
							}
							printf("Sampler set\n");
						}
						else if(uni->type == UniformType::MODEL)
						{
							int location = glGetUniformLocation(tempShaderID,"model");
							if(location == -1 )
							{
								success = false;
								continue;

							}
							printf("Model set\n");
							//glGetUniformLocation()
						}
						else
						{
							if(!setup_uniform(uni,tempShaderID)){
								success = false;
								continue;
							}
							printf("Normal uni set\n");
						}
					}
					if(BIT_CHECK(i->globalUniformFlags,GlobalUniforms::MVP) && success)
					{
						if(!set_global_uniform_to_program(tempShaderID,"MatrixBlock",
									MATRIXES_UNIFORM_LOC))
						{
							success = false;
						}
					}
					if(BIT_CHECK(i->globalUniformFlags,GlobalUniforms::GlobalLight) && success)
					{
						//printf("SET LIGHTS");
						//uint lightIndex = glGetUniformBlockIndex(*idIter, "GlobalLight");   
						//glUniformBlockBinding(*idIter, lightIndex, GLOBALLIGHT_UNIFORM_LOC);
						//glCheckError();
						if(!set_global_uniform_to_program(tempShaderID,"GlobalLight",
									GLOBALLIGHT_UNIFORM_LOC))
						{
							success = false;
						}
					}

					if(success)
					{
						printf("loading succeed \n");
						glDeleteProgram(manager->shaderProgramIds[index]);
						manager->shaderProgramIds[index] = tempShaderID;
						memcpy(i->uniforms,tempArray,sizeof(UniformInfo) * i->numUniforms);
					}
					else{
						glDeleteProgram(tempShaderID);
						printf("loading failed \n");
					}

					//static inline bool setup_uniform_sampler2D(UniformInfo* uniform,uint id,int numTex)
					//static inline bool setup_uniform(UniformInfo* uniform,uint id)
				}
			}
		}
	}
}
//}
//TODO modelille oma chekki!

#endif// PAKKI_SHADERS
