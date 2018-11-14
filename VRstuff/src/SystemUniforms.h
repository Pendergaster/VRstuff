#ifndef PAKKI_SYSTEMUNIFORMS
#define PAKKI_SYSTEMUNIFORMS
#include <glad/glad.h>
#include <Utils.h>
#include <MathUtil.h>
#include "ShaderDefs.h"
#include "glerrorcheck.h"
struct SystemUniforms
{
	uint matrixUniformBufferObject = 0;
	uint globalLightBufferObject = 0;
	uint cameraBlockBufferObject = 0;
};
#define MATRIXES_UNIFORM_LOC 0
#define GLOBALLIGHT_UNIFORM_LOC 1
#define CAMERABLOCK_BUFFER_LOC 2
bool set_global_uniform_to_program(uint program,const char* name,uint location)
{
	uint index = glGetUniformBlockIndex(program,name); 
	//TODO check oikea syntax
	if(index == GL_INVALID_OPERATION ){
		return false;
	}
	glUniformBlockBinding(program, index, location);
	printf("%s \n",name);
	glCheckError();
	return true;
}
void init_systemuniforms(SystemUniforms* rend,ShaderProgram* programs,
		uint* renderIds,uint numPrograms)
{
	uint* idIter = renderIds;
	for(ShaderProgram* i = programs; i < programs + numPrograms; i++,idIter++)
	{
		//if(i->group != RenderGroup::Model) continue;

		if(BIT_CHECK(i->globalUniformFlags,GlobalUniforms::MVP)) 
		{
			//printf("SET MVP");
			//uint matrixIndex = glGetUniformBlockIndex(*idIter, "MatrixBlock");   
			//glUniformBlockBinding(*idIter, matrixIndex, MATRIXES_UNIFORM_LOC);
			//glCheckError();
			if(!set_global_uniform_to_program(*idIter,"MatrixBlock",
						MATRIXES_UNIFORM_LOC))
			{
				ABORT_MESSAGE("Failed to set Matrix Block \n");
			}

		}

		if(BIT_CHECK(i->globalUniformFlags,GlobalUniforms::GlobalLight))
		{
			//printf("SET LIGHTS");
			//uint lightIndex = glGetUniformBlockIndex(*idIter, "GlobalLight");   
			//glUniformBlockBinding(*idIter, lightIndex, GLOBALLIGHT_UNIFORM_LOC);
			//glCheckError();
			if(!set_global_uniform_to_program(*idIter,"GlobalLight",
						GLOBALLIGHT_UNIFORM_LOC))
			{
				ABORT_MESSAGE("Failed to set GlobalLight \n");
			}
		}
#if 0
		if(BIT_CHECK(i->globalUniformFlags,GlobalUniforms::CameraBlock))
		{
			//printf("SET LIGHTS");
			//uint lightIndex = glGetUniformBlockIndex(*idIter, "GlobalLight");   
			//glUniformBlockBinding(*idIter, lightIndex, GLOBALLIGHT_UNIFORM_LOC);
			//glCheckError();
			if(!set_global_uniform_to_program(*idIter,"CameraBlock",
						CAMERABLOCK_BUFFER_LOC))
			{
				ABORT_MESSAGE("Failed to set camera buffer loc \n");
			}
		}
#endif

	}

	{
		glCheckError();
		glGenBuffers(1, &rend->matrixUniformBufferObject);

		glCheckError();
		glBindBuffer(GL_UNIFORM_BUFFER, rend->matrixUniformBufferObject);

		glCheckError();
		glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(MATH::mat4),
				NULL, GL_STATIC_DRAW); 
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glCheckError();
		glBindBufferRange(GL_UNIFORM_BUFFER, MATRIXES_UNIFORM_LOC, 
				rend->matrixUniformBufferObject, 0, 3 * sizeof(MATH::mat4)); 

		glCheckError();
	}
	{
		glCheckError();
		glGenBuffers(1, &rend->globalLightBufferObject);

		glCheckError();
		glBindBuffer(GL_UNIFORM_BUFFER, rend->globalLightBufferObject);

		glCheckError();
		glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(MATH::vec4),
				NULL, GL_STATIC_DRAW); 
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glCheckError();
		glBindBufferRange(GL_UNIFORM_BUFFER, GLOBALLIGHT_UNIFORM_LOC, 
				rend->globalLightBufferObject, 0,4 * sizeof(MATH::vec4)); 

		glCheckError();
	}
#if 0
	{
		glCheckError();
		glGenBuffers(1, &rend->cameraBlockBufferObject);

		glCheckError();
		glBindBuffer(GL_UNIFORM_BUFFER, rend->cameraBlockBufferObject);

		glCheckError();
		glBufferData(GL_UNIFORM_BUFFER, sizeof(MATH::vec4),
				NULL, GL_STATIC_DRAW); 
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glCheckError();
		glBindBufferRange(GL_UNIFORM_BUFFER, CAMERABLOCK_BUFFER_LOC, 
				rend->cameraBlockBufferObject, 0, sizeof(MATH::vec4)); 

		glCheckError();
	}
#endif
	//TODO POJAT SAMAAN BUFFERIIN!
	//glUniformBlockBinding(defaultRendererID, matrixIndex, 2);
}


#endif// PAKKI_SYSTEMUNIFORMS

