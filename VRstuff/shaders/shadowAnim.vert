#version 330 core

#include "shaders/AnimMeshData.inc"

void main()
{
    gl_Position = calculate_glPos();
}


