#include <stdio.h>
#include <glad/glad.h>
#include "glad.c"
#include <shader_utils.h>
int main()
{
	printf("hello world \n");
	char* f,*v;
	SHADER::load_frag_and_vert("test.glsl",&f ,&v);

	printf("%s \n",f);
	printf("%s \n",v);

	return 0;
}
