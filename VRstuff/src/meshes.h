#ifndef PAKKI_MESHES
#define PAKKI_MESHES
#include <Containers.h>
#include <JsonToken.h>
#include <Utils.h>
#include <stb_image.h>
#include <glad/glad.h>
#include <ModelData.h>
#include <file_system.h>
#include "glerrorcheck.h"
#include <meshdefs.h>
//TODO free method for meshes? 
//TODO generoi opengl meshit muualla? 
static inline bool load_mesh(MeshInfo* info,Mesh* data,CONTAINER::MemoryBlock* workingMem)
{
	size_t sizeOfFile = 0;
	void* modelDataDump = FILESYS::load_binary_file_to_block(info->path,workingMem,&sizeOfFile);
	if(!modelDataDump) return false;
	ModelAligment* aligment = (ModelAligment*)modelDataDump;		
	ModelData modelData;
	modelData.vertexes = (MATH::vec3*)(aligment +1);
	modelData.normals = modelData.vertexes + aligment->numVerts;
	modelData.texCoords =(MATH::vec2*)(modelData.normals + aligment->numNormals);
	modelData.indexes = (int*)(modelData.texCoords + aligment->numTextureCoords);
	info->numIndexes = aligment->numIndexes;
	info->numVerts = aligment->numVerts;

	glGenBuffers(4,(uint*)(data));
	glGenVertexArrays(1,&data->vao);
	glBindVertexArray(data->vao);

	glBindBuffer(GL_ARRAY_BUFFER,data->vertBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MATH::vec3) * aligment->numVerts, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MATH::vec3) * aligment->numVerts, modelData.vertexes);

	glBindBuffer(GL_ARRAY_BUFFER,data->normalBuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MATH::vec3) * aligment->numNormals, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MATH::vec3) * aligment->numNormals, modelData.normals);

	glBindBuffer(GL_ARRAY_BUFFER,data->texCoordBuffer);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MATH::vec2) * aligment->numTextureCoords, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MATH::vec2) * aligment->numTextureCoords, modelData.texCoords);
#if 0
	glBindBuffer(GL_ARRAY_BUFFER,data->texCoordBuffer);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MATH::vec2) * aligment->numTextureCoords, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MATH::vec2) * aligment->numTextureCoords, modelData.texCoords);
#endif
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, aligment->numIndexes * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, aligment->numIndexes * sizeof(uint), modelData.indexes);
    glCheckError();
	return true;
}

static void fill_mesh_cache(MeshData* meshData,CONTAINER::MemoryBlock* workingMem,
		CONTAINER::MemoryBlock* staticAllocator)
{
	JsonToken token;
	CONTAINER::DynamicArray<char*> names;
	CONTAINER::init_dynamic_array(&names);
    token.ParseFile("importdata/meshdata.json");
	defer{CONTAINER::dispose_dynamic_array(&names);};
	token.GetKeys(&names);
	//int num = 0;
	meshData->numMeshes = names.numobj;
	CONTAINER::init_table_with_block(&meshData->meshCache,staticAllocator,names.numobj);
	CONTAINER::MemoryBlock lastStateOfMem = *workingMem;
	meshData->meshArray = (Mesh*)CONTAINER::get_next_memory_block(*staticAllocator);
	CONTAINER::increase_memory_block(staticAllocator,names.numobj * sizeof(Mesh));

	meshData->meshInfos = (MeshInfo*)CONTAINER::get_next_memory_block(*staticAllocator);
	CONTAINER::increase_memory_block(staticAllocator,names.numobj * sizeof(Mesh));

	for(uint i = 0; i < names.numobj;i++)
	{
		char* currentName = names.buffer[i];
		LOG("Getting model %s", currentName);
		char* tempName = (char*)CONTAINER::get_next_memory_block(*staticAllocator);
		strcpy(tempName,currentName);
		CONTAINER::increase_memory_block_aligned(staticAllocator,(int)strlen(tempName)+1);
		JsonToken* meshToken = token[currentName].GetToken();
		ASSERT_MESSAGE(meshToken,"MESH TOKEN NOT VALID %s \n",currentName);
		char* metaDataPath = (*meshToken)["metaDataPath"].GetString();
		MeshInfo info;
		info.name = tempName;
		info.path = metaDataPath;

		Mesh data;
		if(!load_mesh(&info,&data,workingMem))
		{
			ABORT_MESSAGE("FAILED TO LOAD MESH :: %s \n",currentName);
		}
		*workingMem = lastStateOfMem;
		CONTAINER::insert_to_table<int>(&meshData->meshCache,currentName,i);
		meshData->meshArray[i] = data;
		meshData->meshInfos[i] = info;

		glBindVertexArray(0);
	}
}
#endif //PAKKI_MESHES
