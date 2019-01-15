#version 330 core

#include "shaders/PakkiMeshData.inc"

void main()
{
    gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
}



