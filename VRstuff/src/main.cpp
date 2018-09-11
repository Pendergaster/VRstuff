#define _CRT_SECURE_NO_WARNINGS
#include <glad/glad.h>
#include "src/glad.c"
//missä poika on
#include "glfw3.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#define SCREENWIDHT 800
#define SCREENHEIGHT 1000

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3),
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow* window = glfwCreateWindow(SCREENWIDHT,SCREENHEIGHT, "Tabula Rasa", NULL, NULL);
	assert(window); // failed to create window
	glfwMakeContextCurrent(window);
	assert((gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)));
	 while (!glfwWindowShouldClose(window)){
			glfwPollEvents(); 
			glClearColor(1.f, 0.f, 0.f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glfwSwapBuffers(window);
	 }
	glfwTerminate();
	return 0;
}
