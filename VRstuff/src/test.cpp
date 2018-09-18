#include <ree.h>
#include <stdio.h>
#include <glad/glad.h>
#include "glad.c"
#include <shader_utils.h>

int main()
{
	printf("hello world %d \n",(int)sizeof(int));
	CONTAINER::MemoryBlock mem;
	CONTAINER::init_memory_block(&mem,10000);

	//char* frag,*vert;
	//SHADER::load_frag_and_vert("shaders/yksi.glsl",&frag,&vert,&mem);
	char* data = SHADER::load_shader("shaders/yksi.glsl",&mem);
	//const char* names[] = {
	//	"yksi","kaksi","kolme"
	//};
	//size_t size = 0;
	//char* data  = FILESYS::load_multiple_files_to_mem_block(names,3,&mem,&size);
	printf("Shader end \n %s \n",data);
	//printf("shader loaded \n %s \n with size %d \n ", data,(int)size);
	//printf("shader loaded \n %s \n %s \n", vert,frag);
	//////////////printf(" %s  \n  ",sha);
	return 0;
}
