#ifndef PAKKI_SHADER_UTILS
#define PAKKI_SHADER_UTILS

#include "Utils.h"
#include "glad/glad.h"
#include "MathUtil.h"
#include "Containers.h"
//#include "material.h"
#include "file_system.h"
#include <cstring>



namespace SHADER
{
	//	uint compile_shader(uint glenum, const char* source);
	//	uint create_program(const GRAPHICS::ShaderProgram& program) 
	//	{
	//		char* vertFile = FILESYS::load_file(program.vertexPath);
	//		char* fragFile = FILESYS::load_file(program.fragmentPath);
	//		uint vertID = compile_shader(vertFile);
	//		uint fragID = compile_shader(fragFile);
	//
	//		free(vertFile);
	//		free(fragFile);
	//		
	//
	//		return 0u;	
	//	}
	void add_attribute(const int program, const char* name,const uint numAttrib)
	{
		LOG("Adding attribute %s to program %d \n", name, program);
		glBindAttribLocation(program,numAttrib, name);
	}

	static bool link_shader(uint program, uint vert, uint frag)
	{
		glAttachShader(program,vert);
		glAttachShader(program,frag);
		glLinkProgram(program);
		int linked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &linked);

		if (!linked)
		{
			int infolen = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infolen);
			if (infolen > 1)
			{
				char* infolog = (char*)malloc(sizeof(char)*infolen);
				glGetProgramInfoLog(program, infolen, NULL, infolog);
				LOG("Error linking program \n %s", infolog);
				free(infolog);
			}
			glDeleteProgram(program);
			return false;
		}

		glDeleteShader(vert);
		glDeleteShader(frag);
		return true;
	}

	bool compile_shader(uint glenum, const char* source,uint* id)
	{
		int compilecheck = 0;
		int shader = glCreateShader(glenum);
		if (shader == 0) return false;

		glShaderSource(shader, 1, &source, NULL);
		glCompileShader(shader);

		glGetShaderiv(shader, GL_COMPILE_STATUS, &(compilecheck));

		if (!compilecheck)
		{
			int infolen = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &(infolen));
			if (infolen > 1)
			{
				char* infoLog = (char*)malloc(sizeof(char) * infolen);
				glGetShaderInfoLog(shader, infolen, NULL, infoLog);
				LOG("Error compiling shader :\n%s\n", infoLog);
				free(infoLog);
				glDeleteShader((GLuint)shader);
				//assert(false);
				return false;

			}
		}
		*id = shader;
		return true;
	}
	inline void set_vec2_name(int program, const char* name, const MATH::vec2 vec)
	{
		uint loc = glGetUniformLocation(program, name);
		ASSERT_MESSAGE(loc != GL_INVALID_INDEX,"VALUE NOT FOUND");
		glUniform2f(loc, vec.x, vec.y);
	}

	inline void set_vec3_name(int program, const char* name, const MATH::vec3 vec)
	{
		uint loc = glGetUniformLocation(program, name);
		ASSERT_MESSAGE(loc != GL_INVALID_INDEX,"VALUE NOT FOUND");
		glUniform3f(loc, vec.x, vec.y, vec.z);
	}

	inline void set_vec3_location(uint loc, const MATH::vec3 vec)
	{
		glUniform3f(loc, vec.x, vec.y, vec.z);
	}

	inline void set_uniform_float_name(int program, const char* name, float value)
	{	
		uint loc = glGetUniformLocation(program, name);
		ASSERT_MESSAGE(loc == GL_INVALID_INDEX,"VALUE NOT FOUND");
		glUniform1f(loc, value);
	}

	inline void set_uniform_float_location(const int loc, float value)
	{	
		glUniform1f(loc, value);
	}

	inline void set_mat4_name(const int program, const char* name,const float mat[4][4])
	{
		uint loc = glGetUniformLocation(program, name);
		ASSERT_MESSAGE(loc != GL_INVALID_INDEX,"VALUE NOT FOUND");
		glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)mat);
	}

	inline void set_mat4_location(const int location,const float mat[4][4])
	{
		glUniformMatrix4fv(location, 1, GL_FALSE, (GLfloat*)mat);
	}


	void set_uniform_int_name(int program, const char* name, int value)
	{
		uint loc = glGetUniformLocation(program, name);
		ASSERT_MESSAGE(loc == GL_INVALID_INDEX,"VALUE NOT FOUND");
		glUniform1i(loc, value);
	}

	void set_uniform_int_location(int location, int value)
	{
		glUniform1i(location, value);
	}

	void set_vec4_location(int location, const MATH::vec4* vec)
	{
		glUniform4f(location, vec->x, vec->y, vec->z,vec->w);
	}

	uint get_uniform_location(int program, const char* name)
	{
		uint loc = glGetUniformLocation(program, name);
		ASSERT_MESSAGE(loc == GL_INVALID_INDEX,"VALUE NOT FOUND");
		return loc;
	}

	static inline bool is_word(char* ptr,char* word)
	{
		int len = (int)strlen(word);
		for(char* i = ptr, *i2 = word; i < ptr + len; i2++,i++)
		{
			if(*i != *i2 || *ptr == '\0')
			{
				return false;
			}
		}
		return true;
	}
	//TODO sana bufferiin 
	static inline void get_quoted_word(char* iter,char* buffer)
	{
		int numWords = 0;
		for(;(*iter) != '\0' &&  (*iter) != '\n';iter++ )	
		{
			if((*iter) != '"' && (*iter) != ' ') 
			{
				buffer[numWords] = (*iter);
				numWords++;
			}
			*iter = ' ';
		}
		buffer[numWords] = '\0';
	}
	//static char* parse_header(char* shader)
	//{
	//	return NULL;
	//}
	//static inline std::tuple<char*,size_t,size_t> 
	static inline char*	parse_header_to_shader(
			const char* rawShader,int rawShaderSize,
			char* startOfIncludesInRawShader,char* startOfIncludesInBlock, size_t sizeOfIncludes,CONTAINER::MemoryBlock* block)
	{
		int startStride = (int)(startOfIncludesInRawShader - rawShader);
		size_t newSize = rawShaderSize + sizeOfIncludes + 1;
		CONTAINER::ensure_memory_block(block,(int)newSize);
#if 0
		if(newSize > (block->size - block->))
		{
			char* temp =*bufferIncludes;
			*bufferIncludes = (char*)realloc(bufferIncludes,newSize);
			if(!(*bufferIncludes))
			{
				*bufferIncludes = temp;
				ABORT_MESSAGE("MEMORY ERROR");
			}
			*allocatedSize = newSize;
		}
#endif

		char* realShaderData = (char*)CONTAINER::get_next_memory_block(*block);
		CONTAINER::increase_memory_block_aligned(block,(int)newSize);
		//move includes down
		memmove(realShaderData + startStride , 
				startOfIncludesInBlock,sizeOfIncludes);
		//move start stride
		memmove(realShaderData, rawShader,startStride);
		//move end to place
		memmove(realShaderData + startStride + sizeOfIncludes
				, rawShader + startStride,rawShaderSize - startStride + 1);
		char* end =  (char*)CONTAINER::get_next_memory_block(*block);
		end--;
		*end = '\0';
		return realShaderData;
	}
	static char * includes_to_shader(char* start,
			char* end,
			char** includes,const int numIncs,
			CONTAINER::MemoryBlock* block);


	static bool	load_frag_and_vert(const char* path,
			char** vertFile,char** fragFile,CONTAINER::MemoryBlock* block)
	{
		size_t rawShaderSize = 0;
		char* rawShader = FILESYS::load_file_to_memblock(path,block,&rawShaderSize);
		int	  numFragmentPeaces = 0;
		char* fragmentPeaces[10];
		int	  numVertexPeaces = 0;
		char* vertexPeaces[10];

		char* fragStart = NULL;
		char* vertStart = NULL;
		char* fragEnd = NULL;
		char* vertEnd = NULL;
		bool insideFrag = 0;
		bool insideVert = 0;
		char* fragStartStr = (char*)"#FragmentStart";
		char* vertStartStr = (char*)"#VertexStart";
		char* fragEndStr = (char*)"#FragmentEnd";
		char* vertEndStr = (char*)"#VertexEnd";
		char* includeStr = (char*)"#include";
		for(char* iter = rawShader; *iter ;iter++)
		{
			switch (*iter)	
			{
				case '#':
					{
						if(is_word(iter,fragStartStr))
						{
							iter += strlen(fragStartStr);
							fragStart = iter;
							insideFrag = true;
						}
						else if(is_word(iter,fragEndStr))
						{
							memset(iter,' ',strlen(fragEndStr));
							iter += strlen(fragEndStr);
							fragEnd = iter;
							insideFrag = false;
						}
						else if(is_word(iter,vertStartStr))
						{
							iter += strlen(vertStartStr); vertStart = iter;
							insideVert = true;
						}
						else if(is_word(iter,vertEndStr))
						{
							memset(iter,' ',strlen(vertEndStr));
							iter += strlen(vertEndStr);
							vertEnd = iter;
							insideVert = false;
						}
						else if(is_word(iter,includeStr))
						{
							memset(iter,' ',strlen(includeStr));
							iter += strlen(includeStr);
							if(insideFrag){
								fragmentPeaces[numFragmentPeaces++] = iter;
							}
							else if(insideVert){
								vertexPeaces[numVertexPeaces++] = iter;
							}
							else{
								ABORT_MESSAGE("SHADER %s INCLUDE FILE ERROR\n",path);
							}
							//iter += strlen("#include");
						}
					}
					break;
				default:
					break;
			}
		}
		if(!fragStart) {
			LOG("IN SHADER %s START NOT DEFINED\n",path);
			return false;
		}
		if(!fragEnd) {
			LOG("IN SHADER %s END NOT DEFINED\n",path);
			return false;
		}
		if(!vertEnd) {
			LOG("IN SHADER %s END NOT DEFINED\n",path);
			return false;
		}
		if(!vertStart) {
			LOG("IN SHADER %s START NOT DEFINED\n",path);
			return false;
		}
		//ASSERT_MESSAGE(fragStart,"IN SHADER %s START NOT DEFINED\n",path);
		//ASSERT_MESSAGE(fragEnd,"IN SHADER %s NO  END DEFINED\n",path);
		//ASSERT_MESSAGE(vertEnd ,"IN SHADER %s NO END DEFINED\n",path);
		//ASSERT_MESSAGE(vertStart ,"IN SHADER %s NO START DEFINED\n",path);

		char* fragSha = NULL,*vertSha = NULL;
		vertSha = includes_to_shader(vertStart,vertEnd,
				vertexPeaces,
				numVertexPeaces,block);
		fragSha = includes_to_shader(fragStart,
				fragEnd,fragmentPeaces,
				numFragmentPeaces,block);

		*vertFile = vertSha;
		*fragFile = fragSha;
		return true;
	}

	static char* load_shader(const char* path
			,CONTAINER::MemoryBlock* block)
	{
		size_t size = 0;
		char* rawFile = FILESYS::load_file_to_memblock(
				path,block,&size);
		int	  numPeaces = 0;
		char* peaces[10];

		char* includeStr = (char*)"#include";
		for(char* iter = rawFile; *iter ;iter++)
		{
			switch (*iter)	
			{
				case '#':
					{
						if(is_word(iter,includeStr))
						{
							memset(iter,' ',strlen(includeStr));
							iter += strlen(includeStr);
							peaces[numPeaces++] = iter;
						}
					}
					break;
				default:
					break;
			}
		}
		char* endFile = rawFile + size;//- 1;
		return includes_to_shader(rawFile , 
				endFile,peaces,numPeaces,block);	
	}

	static char * includes_to_shader(char* start,
			char* end,
			char** includes,const int numIncs,
			CONTAINER::MemoryBlock* block)
	{
		if(numIncs == 0) 
		{
			*end  ='\0';
			return start;
		}
		char* sha = NULL;
		size_t size = 0;
		char* pointersToNames[10];
		char* realNames = 
			(char*)CONTAINER::get_next_memory_block(*block);
		CONTAINER::increase_memory_block_aligned(block,
				numIncs * sizeof(char) * 50);
		int indexTocharBuffer = 0;
		for(int i = 0; i < numIncs;i++)
		{
			pointersToNames[i] = realNames + indexTocharBuffer;
			get_quoted_word(includes[i],pointersToNames[i]);
			//printf("Name got from the %s \n",pointersToNames[i]);
			indexTocharBuffer += (int)strlen(pointersToNames[i]) + 1;
			//realNames[i] 
		}

		sha = FILESYS::load_multiple_files_to_mem_block(
				(const char**)pointersToNames,numIncs,block,&size);
		sha = parse_header_to_shader(start,(int)(end-start),(char*)includes[0],sha,size,block);

		return sha;
	}
}

#endif	// PAKKI_SHADER_UTILS
