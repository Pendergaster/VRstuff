#ifndef PAKKI_MESHES
#define PAKKI_MESHES
#include <Containers.h>
#include <JsonToken.h>
#include <Utils.h>
#include<stb_image.h>
#include <glad/glad.h>

struct Mesh
{
	uint vertBuffer = 0;
	uint normalBuffer = 0;
	uint texCoordBuffer = 0;
	uint indexBuffer = 0;
	uint vao = 0;
};
struct MeshData
{
	uint							numMeshes = 0;
	CONTAINER::StringTable<int>		meshCache;
	Mesh*							meshArray = NULL;
};


static void fill_mesh_cache(MeshData* meshData,CONTAINER::MemoryBlock* workingMem,
CONTAINER::MemoryBlock* staticAllocator)
	//	Mesh* meshArray,int* numMeshes,CONTAINER::StringTable<int>* nameTable,JsonToken* token,
	//	CONTAINER::DynamicArray<char*>* names,CONTAINER::MemoryBlock* workingMem,
	//	CONTAINER::MemoryBlock* staticAllocator)
{
	int num = 0;
	CONTAINER::MemoryBlock lastStateOfMem = *workingMem;
	for(uint i = 0; i < names->numobj;i++)
	{
		char* currentName = names->buffer[i];
		char* tempName = (char*)CONTAINER::get_next_memory_block(*staticAllocator);
		strcpy(tempName,currentName);
		CONTAINER::increase_memory_block_aligned(staticAllocator,strlen(tempName)+1);
		JsonToken* meshToken = (*token)[currentName].GetToken();
		ASSERT_MESSAGE(meshToken , "MESH TOKEN NOT VALID %s \n",currentName);
		char* metaDataPath = (*meshToken)["metaDataPath"].GetString();
		printf("metaDataPath path for %s is %s \n",currentName,metaDataPath);
		size_t sizeOfFile = 0;
		void* modelDataDump = FILESYS::load_binary_file_to_block(metaDataPath,workingMem,&sizeOfFile);
		ModelAligment* aligment = (ModelAligment*)modelDataDump;		
		ModelData modelData;
		modelData.vertexes = (MATH::vec3*)(aligment +1);
		modelData.normals = modelData.vertexes + aligment->numVerts;
		modelData.texCoords =(MATH::vec2*)(modelData.normals + aligment->numNormals);
		modelData.indexes = (int*)(modelData.texCoords + aligment->numTextureCoords);
		Mesh mesh;
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

		glBindBuffer(GL_ARRAY_BUFFER,mesh.texCoordBuffer);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(MATH::vec2) * aligment->numTextureCoords, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MATH::vec2) * aligment->numTextureCoords, modelData.texCoords);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, aligment->numIndexes * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, aligment->numIndexes * sizeof(uint), modelData.indexes);

		*workingMem = lastStateOfMem;
		CONTAINER::insert_to_table<int>(nameTable,currentName,num);
		meshArray[num++] = mesh;

		glBindVertexArray(0);
	}
	*numMeshes = num;
}

#endif //PAKKI_MESHES
