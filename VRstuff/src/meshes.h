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
#define MAX_IMPORT_ANIMATION_KEYS 1000
struct KeyLoader
{
	RotationKey* rotationKeys = NULL;
	PositionKey* positionKeys = NULL;
	ScaleKey*	scaleKeys = NULL;
	void addToLoader(uint numPos,uint numRot,uint numScale)
	{
		positionKeys += numPos;
		rotationKeys += numRot;
		scaleKeys += numScale;
	}
};
static bool load_model(
		ModelInfo* info,
		Mesh* meshArray,
		RenderNode* renderNodes,
		Animation* animations,
		AnimationChannel* animationChannels,
		BoneData* boneDatas,
		KeyLoader* keys,
		CONTAINER::MemoryBlock* workingMem
		)
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
#if 0
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
#endif
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
#if 0
#define MAX_MESH_PARTS 50
#define MAX_MESHES 50
#define MAX_IMPORT_BONES 100
#define MAX_IMPORT_ANIMATIONS 10
#define MAX_IMPORT_ANIMATION_CHANNELS 100
#endif

static void load_mesh(FILE* file,Mesh* mesh,bool animated,
		CONTAINER::MemoryBlock workingMem)
{
	char nameBuffer[100];
	int matches = fscanf(file,"%s \n",nameBuffer);
	ASSERT_MESSAGE(matches == 1, "SYNTAX ERROR \n");
	void* mem = FILESYS::load_binary_file_to_block(nameBuffer,&workingMem,NULL);
	meshAligment* aligment = (meshAligment*)mem;
	mem = VOIDPTRINC(mem,sizeof(meshAligment));
	MATH::vec3* verts = (MATH::vec3*)mem;
	mem = VOIDPTRINC(mem,sizeof(MATH::vec3) * aligment->numVerts);
	MATH::vec3* normals = (MATH::vec3*)mem;
	mem = VOIDPTRINC(mem,sizeof(MATH::vec3) * aligment->numNormals);
	MATH::vec2* uvs = (MATH::vec2*)mem;
	mem = VOIDPTRINC(mem,sizeof(MATH::vec2) * aligment->numTextureCoords);
	int* indexes = (int*)mem;
	mem = VOIDPTRINC(mem,sizeof(int) * aligment->numIndexes);

	glGenBuffers(4,(uint*)(mesh));
	glGenVertexArrays(1,&mesh->vao);
	glBindVertexArray(mesh->vao);

	glBindBuffer(GL_ARRAY_BUFFER,mesh->vertBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MATH::vec3) * aligment->numVerts, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MATH::vec3) * aligment->numVerts, verts);

	glBindBuffer(GL_ARRAY_BUFFER,mesh->normalBuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MATH::vec3) * aligment->numNormals, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MATH::vec3) * aligment->numNormals, normals);

	glBindBuffer(GL_ARRAY_BUFFER,mesh->texCoordBuffer);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MATH::vec2) * aligment->numTextureCoords, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MATH::vec2) * aligment->numTextureCoords, uvs);
	glCheckError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, aligment->numIndexes * sizeof(uint), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, aligment->numIndexes * sizeof(uint), indexes);
	glCheckError();


	if(animated)
	{
		glGenBuffers(1,(uint*)(&mesh->bonedataBuffer));
		glBindBuffer(GL_ARRAY_BUFFER,mesh->bonedataBuffer);
		VertexBoneData* vertbonedata = (VertexBoneData*)mem;

		glVertexAttribPointer(3, MAX_BONES_IN_VERTEX, GL_INT, GL_FALSE,
			   	MAX_BONES_IN_VERTEX * sizeof(int), (void*)offsetof(VertexBoneData,IDs));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(4, MAX_BONES_IN_VERTEX, GL_FLOAT, GL_FALSE,
			   	MAX_BONES_IN_VERTEX * sizeof(float), (void*)offsetof(VertexBoneData,weights));
		glEnableVertexAttribArray(4);

		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexBoneData) * aligment->numBoneData, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0,sizeof(VertexBoneData) * aligment->numBoneData, vertbonedata);

	}

	glBindVertexArray(0);

	//FILE* meshbin = fopen(nameBuffer,"rb");
	//defer{fclose(meshbin);};
	//MeshAl
}

static void fill_mesh_cache(ModelCache* meshData,CONTAINER::MemoryBlock* workingMem,
		CONTAINER::MemoryBlock* staticAllocator)
{

	JsonToken rootToken;
	rootToken.ParseFile("temp/root.info");
	meshData->numMeshes = (uint)rootToken["NumMeshes"].GetInt();
	meshData->numRenderNodes = (uint)rootToken["NumNodes"].GetInt();
	meshData->numBones = (uint)rootToken["NumBones"].GetInt();
	meshData->numAnimations = (uint)rootToken["numAnimations"].GetInt();
	meshData->numAnimationsChannels = (uint)rootToken["NumAnimationChannels"].GetInt();
	meshData->numModels = (uint)rootToken["NumModels"].GetInt();

	KeyLoader keys;
	uint numPositionKeys = (uint)rootToken["NumPositionKeys"].GetInt();
	uint numRotationKeys = (uint)rootToken["NumRotationKeys"].GetInt();
	uint numScaleKeys = (uint)rootToken["NumScaleKeys"].GetInt();
	CONTAINER::MemoryBlock lastStateOfMem = *workingMem;

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

	meshData->modelInfos = (ModelInfo*)CONTAINER::get_next_memory_block(*staticAllocator);
	CONTAINER::increase_memory_block(staticAllocator,meshData->numModels * sizeof(ModelInfo));

	meshData->bones = (BoneData*)CONTAINER::get_next_memory_block(*staticAllocator);
	CONTAINER::increase_memory_block(staticAllocator,meshData->numBones * sizeof(BoneData));

	meshData->animations = (Animation*)CONTAINER::get_next_memory_block(*staticAllocator);
	CONTAINER::increase_memory_block(staticAllocator,meshData->numAnimations * sizeof(Animation));

	meshData->animationChannels = (AnimationChannel*)CONTAINER::get_next_memory_block(*staticAllocator);
	CONTAINER::increase_memory_block(staticAllocator,meshData->numAnimationsChannels * sizeof(AnimationChannel));

	meshData->positionKeys = (PositionKey*)CONTAINER::get_next_memory_block(*staticAllocator);
	CONTAINER::increase_memory_block(staticAllocator,numPositionKeys * sizeof(PositionKey));
	keys.positionKeys  = meshData->positionKeys;

	meshData->rotationKeys = (RotationKey*)CONTAINER::get_next_memory_block(*staticAllocator);
	CONTAINER::increase_memory_block(staticAllocator, numRotationKeys * sizeof(RotationKey));
	keys.rotationKeys  = meshData->rotationKeys;

	meshData->scaleKeys = (ScaleKey*)CONTAINER::get_next_memory_block(*staticAllocator);
	CONTAINER::increase_memory_block(staticAllocator, numScaleKeys * sizeof(ScaleKey));
	keys.scaleKeys = meshData->scaleKeys;

	Mesh* meshArray = (Mesh*)CONTAINER::get_next_memory_block(*staticAllocator);
	CONTAINER::increase_memory_block(staticAllocator,meshData->numMeshes * sizeof(Mesh));

	uint writtenNodes = 0,writtenMeshes = 0,writtenAnimations = 0,writtenAnimChannels = 0,writtenBones;
	char nameBuffer[100];
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
		bool animated = (*meshToken)["Animated"].GetBool();
		tempName = (char*)CONTAINER::get_next_memory_block(*staticAllocator);
		strcpy(tempName,metaDataPath);
		CONTAINER::increase_memory_block_aligned(staticAllocator,(int)strlen(metaDataPath)+1);

		info.path = tempName;
		info.meshLoc = writtenMeshes;
		info.renderNodesLoc = writtenNodes;
		info.numAnimations = 0; // todo set
		info.animationLoc = writtenAnimations;
		info.boneLoc = writtenBones;

		FILE* infoFile =  fopen(info.path,"r");
		defer{fclose(infoFile);};
		ASSERT_MESSAGE(infoFile,"INFO FILE NOT FOUND %s",info->name);
		uint numMeshes = 0;
		int matches = fscanf(infoFile,"%d \n",&numMeshes);
		ASSERT_MESSAGE(matches == 1 && numMeshes > 0,
				"INCORRECT SYNTAX :: %s",info.name);
		info.numMeshes = numMeshes;


		for(uint meshIter = 0; meshIter < info.numMeshes;meshIter++)
		{
			load_mesh(infoFile,&meshData->meshArray[info.meshLoc + meshIter],animated,*workingMem);
		}
		writtenMeshes += info.numMeshes;


		if(!load_model(&info,
					&meshData->meshArray[info.meshLoc],
					&meshData->renderNodes[info.renderNodesLoc],
					&meshData->animations[info.animationLoc],
					&meshData->animationChannels[writtenAnimChannels],
					&meshData->bones[writtenBones],
					&keys,
					workingMem))
		{
			ABORT_MESSAGE("FAILED TO LOAD MESH :: %s \n",currentName);
		}

		CONTAINER::insert_to_table<int>(&meshData->meshCache,currentName,i);
		glBindVertexArray(0);
	}
}
#endif //PAKKI_MESHES
