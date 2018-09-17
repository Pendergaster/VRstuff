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
	printf(" %s \n ---------- \n %s  \n ------- \n  ",frag,vert);
	printf("tanne kaatuu");
	char* sha = SHADER::load_shader("yksi.glsl");
	printf("shader loaded \n");
	printf(" %s  \n  ",sha);
	return 0;
}
