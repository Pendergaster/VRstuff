#include <ree.h>
#include <stdio.h>
#include <glad/glad.h>
#include "glad.c"
#include <shader_utils.h>

int main()
{
	printf("hello world \n");
	char* frag,*vert;
	SHADER::load_frag_and_vert("test.glsl",&frag,&vert);
	printf(" %s \n %s  \n  ",frag,vert);
	char* sha = SHADER::load_shader("yksi.glsl");
	printf(" %s  \n  ",sha);
	return 0;
}
