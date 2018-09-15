#include <stdio.h>
#include <glad/glad.h>
#include "glad.c"
#include <shader_utils.h>
int main()
{
	printf("hello world \n");
	SHADER::parse_frag_and_vert("test.glsl");
	return 0;
}
