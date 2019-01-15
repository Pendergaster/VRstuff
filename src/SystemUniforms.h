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
	uint shadowBlockBufferObject = 0;
};
#define MATRIXES_UNIFORM_LOC 0
#define GLOBALLIGHT_UNIFORM_LOC 1
#define CAMERABLOCK_BUFFER_LOC 2
#define SHADOW_BUFFER_LOC 3

static const uint sizeOfShadowBlock = NUM_CASCADES * (sizeof(MATH::mat4) + sizeof(float));
static const uint sizeOfMatrixBlock = 3 * sizeof(MATH::mat4);
static const uint sizeOfGlobalLightBlock = 4 * sizeof(MATH::vec4);

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
			if(!set_global_uniform_to_program(*idIter,"MatrixBlock",
						MATRIXES_UNIFORM_LOC))
			{
				ABORT_MESSAGE("Failed to set Matrix Block \n");
			}

		}

		if(BIT_CHECK(i->globalUniformFlags,GlobalUniforms::GlobalLight))
		{
			if(!set_global_uniform_to_program(*idIter,"GlobalLight",
						GLOBALLIGHT_UNIFORM_LOC))
			{
				ABORT_MESSAGE("Failed to set GlobalLight \n");
			}
		}
		if(BIT_CHECK(i->globalUniformFlags,GlobalUniforms::ShadowBlock))
		{
			if(!set_global_uniform_to_program(*idIter,"ShadowBlock",
						SHADOW_BUFFER_LOC))
			{
				ABORT_MESSAGE("Failed to set shadowblock \n");
			}
		}
	}



	{
		glCheckError();
		glGenBuffers(1, &rend->matrixUniformBufferObject);

		glCheckError();
		glBindBuffer(GL_UNIFORM_BUFFER, rend->matrixUniformBufferObject);

		glCheckError();
		glBufferData(GL_UNIFORM_BUFFER, sizeOfMatrixBlock,
				NULL, GL_STATIC_DRAW); 
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glCheckError();
		glBindBufferRange(GL_UNIFORM_BUFFER, MATRIXES_UNIFORM_LOC, 
				rend->matrixUniformBufferObject, 0, sizeOfMatrixBlock);

		glCheckError();
	}
	{
		glCheckError();
		glGenBuffers(1, &rend->globalLightBufferObject);

		glCheckError();
		glBindBuffer(GL_UNIFORM_BUFFER, rend->globalLightBufferObject);

		glCheckError();
		glBufferData(GL_UNIFORM_BUFFER, sizeOfGlobalLightBlock,
				NULL, GL_STATIC_DRAW); 
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glCheckError();
		glBindBufferRange(GL_UNIFORM_BUFFER, GLOBALLIGHT_UNIFORM_LOC, 
				rend->globalLightBufferObject, 0,
				sizeOfGlobalLightBlock);
		glCheckError();
	}

	{
		glCheckError();
		glGenBuffers(1, &rend->shadowBlockBufferObject);

		glCheckError();
		glBindBuffer(GL_UNIFORM_BUFFER, rend->shadowBlockBufferObject);

		glCheckError();
		glBufferData(GL_UNIFORM_BUFFER, 
				sizeOfShadowBlock,
				NULL, GL_STATIC_DRAW); 
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glCheckError();
		glBindBufferRange(GL_UNIFORM_BUFFER, SHADOW_BUFFER_LOC, 
				rend->shadowBlockBufferObject, 0,sizeOfShadowBlock); 

		glCheckError();
	}
	//TODO POJAT SAMAAN BUFFERIIN!
	//glUniformBlockBinding(defaultRendererID, matrixIndex, 2);
}


#endif// PAKKI_SYSTEMUNIFORMS

