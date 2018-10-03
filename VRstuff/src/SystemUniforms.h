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
};
#define MATRIXES_UNIFORM_LOC 0
#define GLOBALLIGHT_UNIFORM_LOC 1
bool set_global_uniform_to_program(uint program,const char* name,uint location)
{
	uint index = glGetUniformBlockIndex(program,name); 
	//TODO check oikea syntax
	if(index == GL_INVALID_OPERATION ){
		return false;
	}
	glUniformBlockBinding(program, index, location);
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
	}

	{
		glCheckError();
		glGenBuffers(1, &rend->matrixUniformBufferObject);

		glCheckError();
		glBindBuffer(GL_UNIFORM_BUFFER, rend->matrixUniformBufferObject);

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

	//glUniformBlockBinding(defaultRendererID, matrixIndex, 2);
}


#endif// PAKKI_SYSTEMUNIFORMS

