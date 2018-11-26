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
#include <stdlib.h>
#include <string.h>

//TODO free method for meshes? 
//TODO generoi opengl meshit muualla? 
static bool load_model(ModelInfo* info,Mesh* meshArray,MeshPart* partArray,
		CONTAINER::MemoryBlock* workingMem)
{
	FILE* infoFile =  fopen(info->path,"r");
	defer{fclose(infoFile);};
	ASSERT_MESSAGE(infoFile,"INFO FILE NOT FOUND %s",info->name);
	uint numMeshes = 0;
	int matches = fscanf(infoFile,"%d \n",&numMeshes);
	ASSERT_MESSAGE(matches == 1 && numMeshes > 0,
			"INCORRECT SYNTAX :: %s",info->name);
	info->numMeshes = numMeshes;
	char nameBuffer[52];
	for(uint i = 0; i < info->numMeshes; i++) //read meshdata
	{
		//char* currentFilePath = NULL;
		matches = fscanf(infoFile,"%s \n",nameBuffer);
		ASSERT_MESSAGE(matches == 1, "INCORRECT SYNTAX :: %s",info->name);
		CONTAINER::MemoryBlock lastState = *workingMem;
		defer {*workingMem = lastState;};
		size_t sizeOfFile = 0;
		void* modelDataDump = 
			FILESYS::load_binary_file_to_block(nameBuffer,
					workingMem,&sizeOfFile);

		if(!modelDataDump) return false;
		meshAligment* aligment = (meshAligment*)modelDataDump;		
		meshData modelData;
		modelData.vertexes = (MATH::vec3*)(aligment +1);
		modelData.normals = modelData.vertexes + aligment->numVerts;
		modelData.texCoords =(MATH::vec2*)(modelData.normals + 
				aligment->numNormals);
		modelData.indexes = (int*)(modelData.texCoords + aligment->numTextureCoords);
		Mesh mesh;
		mesh.numIndexes = aligment->numIndexes;

		glGenBuffers(4,(uint*)(&mesh));
		glGenVertexArrays(1,&mesh.vao);
		glBindVertexArray(mesh.vao);

		glBindBuffer(GL_ARRAY_BUFFER,mesh.vertBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBufferData(GL_ARRAY_BUFFER, sizeof(MATH::vec3) * aligment->numVerts, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MATH::vec3) * aligment->numVerts, modelData.vertexes);

		glBindBuffer(GL_ARRAY_BUFFER,mesh.normalBuffer);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glBufferData(GL_ARRAY_BUFFER, sizeof(MATH::vec3) * aligment->numNormals, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MATH::vec3) * aligment->numNormals, modelData.normals);

		glBindBuffer(GL_ARRAY_BUFFER,mesh.texCoordBuffer);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(MATH::vec2) * aligment->numTextureCoords, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MATH::vec2) * aligment->numTextureCoords, modelData.texCoords);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, aligment->numIndexes * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, aligment->numIndexes * sizeof(uint), modelData.indexes);
		glCheckError();

		meshArray[i] = mesh;
	}
	//char* modelPartFilePath = NULL;
	matches = fscanf(infoFile,"%s",nameBuffer);
	ASSERT_MESSAGE(matches == 1 && nameBuffer, "INCORRECT SYNTAX :: %s",info->name);
	CONTAINER::MemoryBlock lastState = *workingMem;
	defer {*workingMem = lastState;};
	void* parts = FILESYS::load_binary_file_to_block(nameBuffer,workingMem,nullptr);
	uint numparts = *(uint*)parts;
	parts = VOIDPTRINC(parts,sizeof(uint));
	info->numParts = numparts;
	for(uint i = 0; i < numparts; i++)
	{
		partArray[i] = *(MeshPart*)parts;
		parts = VOIDPTRINC(parts,sizeof(MeshPart));
	}

#if 0
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
#endif
	return true;
}
#define MAX_MESH_PARTS 50
#define MAX_MESHES 50
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
	//meshData->numMeshes = names.numobj;
	CONTAINER::init_table_with_block(&meshData->meshCache,staticAllocator,names.numobj);

	//meshData->meshArray = (Mesh*)CONTAINER::get_next_memory_block(*staticAllocator);
	//CONTAINER::increase_memory_block(staticAllocator,names.numobj * sizeof(Mesh));

	meshData->numInfos = 0;
	meshData->meshInfos = (ModelInfo*)CONTAINER::get_next_memory_block(*staticAllocator);
	CONTAINER::increase_memory_block(staticAllocator,names.numobj * sizeof(ModelInfo));

	meshData->numParts = 0;
	MeshPart* partArray = (MeshPart*)CONTAINER::get_next_memory_block(*workingMem);
	CONTAINER::increase_memory_block(workingMem,MAX_MESH_PARTS * sizeof(MeshPart));
	//meshData->meshParts = CONTAINER::get_next_memory_block(*workingMem);
	//CONTAINER::increase_memory_block(workingMem,MAX_MESH_PARTS * sizeof(MeshPart));


	//meshData->meshParts = (Mesh*)CONTAINER::get_next_memory_block(*workingMem);
	//CONTAINER::increase_memory_block(workingMem,MAX_MESH_PARTS * sizeof(MeshPart));
	meshData->numMeshes = 0;
	Mesh* meshArray = (Mesh*)CONTAINER::get_next_memory_block(*workingMem);
	CONTAINER::increase_memory_block(workingMem,MAX_MESHES * sizeof(Mesh));



	CONTAINER::MemoryBlock lastStateOfMem = *workingMem;
	for(uint i = 0; i < names.numobj;i++)
	{
		ModelInfo info;
		char* currentName = names.buffer[i];
		LOG("Getting model %s", currentName);
		char* tempName = (char*)CONTAINER::get_next_memory_block(*staticAllocator);
		strcpy(tempName,currentName);
		CONTAINER::increase_memory_block_aligned(staticAllocator,(int)strlen(tempName)+1);
		info.name = tempName;
		JsonToken* meshToken = token[currentName].GetToken();
		ASSERT_MESSAGE(meshToken,"MESH TOKEN NOT VALID %s \n",currentName);

		char* metaDataPath = (*meshToken)["metaDataPath"].GetString();
		ASSERT_MESSAGE(metaDataPath ,"MODEL DOES NOT HAVE ENOUGH DATA :: %s \n",currentName);
		tempName = (char*)CONTAINER::get_next_memory_block(*staticAllocator);
		strcpy(tempName,metaDataPath);
		CONTAINER::increase_memory_block_aligned(staticAllocator,(int)strlen(metaDataPath)+1);

		info.path = tempName;
		info.meshLoc = meshData->numMeshes;
		info.partsLoc = meshData->numParts;

		if(!load_model(&info,&meshArray[info.meshLoc],
					&partArray[info.partsLoc],workingMem))
		{
			ABORT_MESSAGE("FAILED TO LOAD MESH :: %s \n",currentName);
		}
		meshData->numMeshes += info.numMeshes;
		meshData->numParts += info.numParts;

		*workingMem = lastStateOfMem;
		CONTAINER::insert_to_table<int>(&meshData->meshCache,currentName,i);
		meshData->meshInfos[i] = info;

		glBindVertexArray(0);
	}
	meshData->meshArray = (Mesh*)CONTAINER::get_next_memory_block(*staticAllocator);
	CONTAINER::increase_memory_block(staticAllocator,sizeof(Mesh) * meshData->numMeshes);
	memcpy(meshData->meshArray,meshArray,sizeof(Mesh) * meshData->numMeshes);

	meshData->meshParts = (MeshPart*)CONTAINER::get_next_memory_block(*staticAllocator);
	CONTAINER::increase_memory_block(staticAllocator,sizeof(MeshPart) * meshData->numParts);
	memcpy(meshData->meshParts,partArray,sizeof(MeshPart) * meshData->numParts);
}
#endif //PAKKI_MESHES
