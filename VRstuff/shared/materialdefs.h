#ifndef PAKKI_MATERIAL_DEFS
#define PAKKI_MATERIAL_DEFS
#include <Utils.h>
#include "ShaderDefs.h"

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
	ASSERT_MESSAGE(uni->type == UniformType::SAMPLER2D,"UNIFORM IS NOT CORRECT TYPE :: CORRECT IS %s\n", UNIFORM_TYPE_NAMES[uni->type] );
	uni->_textureCacheId = shaderCacheID;
}
#endif
