#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ModelData.h>
#include <JsonToken.h>
#include <file_system.h>
#include <Containers.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#define MAX_VERT_AMOUNT 30000

struct objIndex
{
	int vertIndex = 0;
	int normIndex = 0;
	int texIndex = 0;
};

int main()
{
#if 0
	printf("hello world %d \n",(int)sizeof(int));
	FILE* fp = fopen("testData","wb");
	int foo[] = {10,11,12,13};
	fwrite(foo,sizeof(int),4,fp);
	fwrite(foo,sizeof(int),4,fp);
	fwrite(foo,sizeof(int),4,fp);
	fwrite(foo,sizeof(int),4,fp);
	fclose(fp);
	CONTAINER::MemoryBlock block;
	CONTAINER::init_memory_block(&block,10000);
	size_t size = 0;
	void* ptr = FILESYS::load_binary_file_to_block((char*)"testData",&block,&size);
	printf("%d \n",(int)size / (int)sizeof(int));
	printf("%d %d %d %d \n",*((int*)ptr),*((int*)ptr + 1),*((int*)ptr + 2),*((int*)ptr + 3));
	printf("%d %d %d %d \n",*((int*)ptr + 4),*((int*)ptr + 5),*((int*)ptr + 6),*((int*)ptr + 7));
#endif
	//verts - normals - inds
#if 0
	void* rawMem = malloc(MAX_VERT_AMOUNT * (sizeof(MATH::vec3) * 2 + sizeof(objIndex) + sizeof(MATH::vec2)));
	defer {free(rawMem);};
	MATH::vec3* vertexBuffer = (MATH::vec3*)rawMem;
	MATH::vec3* normalBuffer = (MATH::vec3*)rawMem + MAX_VERT_AMOUNT;
	MATH::vec2* textureCoordBuffer = (MATH::vec2*)((MATH::vec3*)rawMem + MAX_VERT_AMOUNT * 2);
	objIndex* indexBuffer = (objIndex*)((MATH::vec2*)((MATH::vec3*)rawMem + MAX_VERT_AMOUNT * 2)+ MAX_VERT_AMOUNT);

	void* outputMem = malloc(MAX_VERT_AMOUNT * (sizeof(MATH::vec3) * 2 + sizeof(int) + sizeof(MATH::vec2)));
	defer {free(outputMem);};
	MATH::vec3* vertexBufferOutput = (MATH::vec3*)outputMem;
	MATH::vec3* normalBufferOutput = (MATH::vec3*)outputMem + MAX_VERT_AMOUNT;
	MATH::vec2* textureCoordBufferOutput = (MATH::vec2*)(normalBufferOutput + MAX_VERT_AMOUNT);
	int* indexBufferOutput = (int*)(textureCoordBufferOutput + MAX_VERT_AMOUNT);



	JsonToken modelToken;
	modelToken.ParseFile("importdata/meshdata.json");
	CONTAINER::DynamicArray<char*> modelNames;
	modelToken.GetKeys(&modelNames);
	for(uint i = 0; i < modelNames.numobj;i++)
	{
		char* currentName = modelNames.buffer[i];
		printf("Parsing model %d/%d :: %s \n",i,modelNames.numobj,currentName);
		JsonToken* currentToken = modelToken[currentName].GetToken();
		ASSERT_MESSAGE(currentToken,"MODELDATA IS NOT VALID ::  %s \n",currentName);
		char* path = (*currentToken)["rawPath"].GetString();
		ASSERT_MESSAGE(path,"RAWPATH IS NOT FOUND ::  %s \n",currentName);
		FILE* fp = fopen(path,"r");
		defer {fclose(fp);};
		ASSERT_MESSAGE(fp,"RAW FILE IS NOT FOUND FOR WITH PATH %s :: %s \n",path,currentName);

		char buff[255];
		int end = fscanf(fp, "%s", buff);

		int numVerts = 0,numNormals = 0,numTextureCoords = 0, numIndexes = 0;
		MATH::vec3* currentVertex = vertexBuffer, *currentNormal = normalBuffer;
		MATH::vec2* currentTextureCoord = textureCoordBuffer;
		objIndex* currentIndex = indexBuffer;

		while(end != EOF)
		{
			if(!strcmp("v",buff))	
			{
				ASSERT_MESSAGE(numVerts + 1 < MAX_VERT_AMOUNT,"VERTEX AMOUNT EXCEEDED \n");
				int matches = fscanf(fp, "%f %f %f\n", &currentVertex->x, &currentVertex->y, &currentVertex->z);
				ASSERT_MESSAGE(matches == 3,"NUMBERS OF VERTS IS NOT CORRECT \n");
				numVerts++;currentVertex++;
			}
			else if(!strcmp("vt",buff))	
			{
				ASSERT_MESSAGE(numTextureCoords + 1 < MAX_VERT_AMOUNT,"TEXTURECOORD AMOUNT EXCEEDED \n");
				int matches = fscanf(fp, "%f %f\n", &currentTextureCoord->x, &currentTextureCoord->y);
				ASSERT_MESSAGE(matches == 2,"NUMBERS OF TEXTURE COORDS IS NOT CORRECT \n");
				numTextureCoords++;currentTextureCoord++;
			}
			else if(!strcmp("vn",buff))	
			{
				ASSERT_MESSAGE(numNormals + 1 < MAX_VERT_AMOUNT,"NORMAL AMOUNT EXCEEDED \n");
				int matches = fscanf(fp, "%f %f %f\n", &currentNormal->x, &currentNormal->y, &currentNormal->z);
				ASSERT_MESSAGE(matches == 3,"NUMBERS OF NORMALS IS NOT CORRECT \n");
				numNormals++;currentNormal++;

			}
			else if(!strcmp("f",buff))	
			{
				ASSERT_MESSAGE(numIndexes + 3 < MAX_VERT_AMOUNT,"INDEX AMOUNT EXCEEDED \n");
				int matches = fscanf(fp, "%d/%d/%d", &currentIndex->vertIndex,&currentIndex->texIndex,&currentIndex->normIndex);
				ASSERT_MESSAGE(matches == 3,"NUMBERS OF NORMALS IS NOT CORRECT \n");
				numIndexes++;currentIndex++;

				matches = fscanf(fp, "%d/%d/%d", &currentIndex->vertIndex,&currentIndex->texIndex,&currentIndex->normIndex);
				ASSERT_MESSAGE(matches == 3,"NUMBERS OF NORMALS IS NOT CORRECT \n");
				numIndexes++;currentIndex++;

				matches = fscanf(fp, "%d/%d/%d\n", &currentIndex->vertIndex,&currentIndex->texIndex,&currentIndex->normIndex);
				ASSERT_MESSAGE(matches == 3,"NUMBERS OF NORMALS IS NOT CORRECT \n");
				numIndexes++;currentIndex++;
			}
			end = fscanf(fp, "%s", buff);
		}
		//int* realIndexArray = (int*)calloc(MAX_VERT_AMOUNT,sizeof(int));

		int numRealIndexes = 0;
		int currentMaxIndex = 0;
		MATH::vec3* vertexIterator = vertexBufferOutput;
		MATH::vec3* normalIterator = normalBufferOutput;
		MATH::vec2* texIterator = textureCoordBufferOutput;
		//jos edellistä vastaavaa vertexiä ei löydy tee uus vertexi ja kasvata indexiarrayta
		//jos löytyy kasvata vastaavalla indexillä lisää 
		for(objIndex* i = indexBuffer; i < (indexBuffer + numIndexes); i++)
		{
			bool indexExists = false;
			int* i2 = indexBufferOutput; //1 1 1 , 2 ,  1 , 1 3
			for(;i2 < (indexBufferOutput + numRealIndexes);i2++ ) // katso onko vastaavaa indeksiä kirjotettu out puttiin
			{
				objIndex* comp = &indexBuffer[*i2];
				if(comp->vertIndex == i->vertIndex && comp->texIndex == i->texIndex &&  comp->normIndex == i->normIndex ) // vertaa outout index buff arvoa
				{
					indexExists = true;
					break;
				}
			}
			if(indexExists)
			{

				indexBufferOutput[numRealIndexes++] = *i2;
			}
			else
			{
				indexBufferOutput[numRealIndexes++] = currentMaxIndex;
				++currentMaxIndex;
				*vertexIterator = vertexBuffer[i->vertIndex - 1];
				vertexIterator++;
				*normalIterator = normalBuffer[i->normIndex - 1];
				normalIterator++;
				*texIterator = textureCoordBuffer[i->texIndex - 1];
				texIterator++;
			}
		}
		char* metaDataPath = (*currentToken)["metaDataPath"].GetString();
		ASSERT_MESSAGE(metaDataPath,"METADATA PATH NOT FOUND FOR :: %s \n",currentName);
		ModelAligment aligment; 
		aligment.numVerts = currentMaxIndex;
		aligment.numNormals = currentMaxIndex;
		aligment.numTextureCoords = currentMaxIndex;
		aligment.numIndexes = numRealIndexes;
		//for(int i2 = 0; i2 < currentMaxIndex;i2++)
		//{
		//	printf("verts %f , %f , %f \n",vertexBufferOutput[i2].x,vertexBufferOutput[i2].y,vertexBufferOutput[i2].z);
		//	printf("norms %f , %f , %f \n",normalBufferOutput[i2].x,normalBufferOutput[i2].y,normalBufferOutput[i2].z);
		//	printf("texs %f , %f \n",textureCoordBufferOutput[i2].x,textureCoordBufferOutput[i2].y);
		//}
		FILE* metaFile = fopen(metaDataPath,"wb");
		defer {fclose(metaFile);};
		fwrite(&aligment,sizeof(ModelAligment),1,metaFile);
		fwrite(vertexBufferOutput,sizeof(MATH::vec3),currentMaxIndex,metaFile);
		fwrite(normalBufferOutput,sizeof(MATH::vec3),currentMaxIndex,metaFile);
		fwrite(textureCoordBufferOutput,sizeof(MATH::vec2),currentMaxIndex,metaFile);
		fwrite(indexBufferOutput,sizeof(int),numRealIndexes,metaFile);
		
		printf("Parsed :: %s with %d Vertexes and %d Indexes \n",currentName,currentMaxIndex,numRealIndexes);
	}
	//for(uint i = 0; i < modelNames.numobj; i++)
	//{

	//}
#endif
#if 1

    JsonToken modelToken;
    modelToken.ParseFile("importdata/meshdata.json");
    CONTAINER::DynamicArray<char*> modelNames;
    CONTAINER::init_dynamic_array(&modelNames);
    defer{ CONTAINER::dispose_dynamic_array(&modelNames); };;
    modelToken.GetKeys(&modelNames);

    Assimp::Importer importer;
    CONTAINER::MemoryBlock block;
    CONTAINER::init_memory_block(&block, 100000);
    for (uint i = 0; i < modelNames.numobj; i++)
    {
        char* currentName = modelNames.buffer[i];;
        printf("Parsing model %d/%d :: %s \n", i, modelNames.numobj, currentName);
        JsonToken* currentToken = modelToken[currentName].GetToken();
		ASSERT_MESSAGE(currentToken,"MODELDATA IS NOT VALID ::  %s \n",currentName);
		char* path = (*currentToken)["rawPath"].GetString();
		ASSERT_MESSAGE(path,"RAWPATH IS NOT FOUND ::  %s \n",currentName);
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
        //const char * error = importer.GetErrorString();
        ASSERT_MESSAGE(scene && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mRootNode, "FAILED TO PARSE :: %s ERROR :: %s \n", currentName, importer.GetErrorString());

        aiNode* current = scene->mRootNode;
       
        //m_Entries meshData;
        const aiMesh* mesh = scene->mMeshes[0];
       // for (;;)
        
            MATH::vec3* pos = (MATH::vec3*)CONTAINER::get_next_memory_block(block);
            CONTAINER::increase_memory_block(&block, mesh->mNumVertices * sizeof(MATH::vec3));
            MATH::vec3* normal = (MATH::vec3*)CONTAINER::get_next_memory_block(block);
            CONTAINER::increase_memory_block(&block, mesh->mNumVertices * sizeof(MATH::vec3));
            MATH::vec2* uv = (MATH::vec2*)CONTAINER::get_next_memory_block(block);
            CONTAINER::increase_memory_block(&block, mesh->mNumVertices * sizeof(MATH::vec2));
            int* index = (int*)CONTAINER::get_next_memory_block(block);
            int maxIndexSize = block.size - block.currentIndex;
            //CONTAINER::increase_memory_block(&block, mesh->mNumVertices * sizeof(float));

            ASSERT_MESSAGE(mesh->mTextureCoords[0],"NO TEXTURE COORDINATES IN :: %s \n",currentName);
            for (unsigned int i2 = 0; i2 < mesh->mNumVertices; i2++)
            {
                pos[i2].x = mesh->mVertices[i2].x;
                pos[i2].y = mesh->mVertices[i2].y;
                pos[i2].z = mesh->mVertices[i2].z;

                normal[i2].x = mesh->mNormals[i2].x;
                normal[i2].y = mesh->mNormals[i2].y;
                normal[i2].z = mesh->mNormals[i2].z;

                uv[i2].x = mesh->mTextureCoords[0][i2].x;
                uv[i2].y = mesh->mTextureCoords[0][i2].y;
            }
            //memcpy(pos, mesh->mVertices, mesh->mNumVertices * sizeof(MATH::vec3));
            //memcpy(normal, mesh->mNormals, mesh->mNumVertices * sizeof(MATH::vec3));
            //memcpy(uv, mesh->mTextureCoords[0], mesh->mNumVertices * sizeof(MATH::vec2));
            uint indexIndex = 0;
            for (unsigned int i2 = 0; i2 < mesh->mNumFaces; i2++)
            {
                aiFace face = mesh->mFaces[i2];
                for (unsigned int i3 = 0; i3 < face.mNumIndices; i3++)
                {
                    CONTAINER::ensure_memory_block(&block, indexIndex * sizeof(int));
                    index[indexIndex++] = face.mIndices[i3];
                }
            }
            char* metaDataPath = (*currentToken)["metaDataPath"].GetString();
            ASSERT_MESSAGE(metaDataPath, "METADATA PATH NOT FOUND FOR :: %s \n", currentName);
            ModelAligment aligment;
            aligment.numVerts = mesh->mNumVertices;
            aligment.numNormals = mesh->mNumVertices;
            aligment.numTextureCoords = mesh->mNumVertices;
            aligment.numIndexes = indexIndex;

            FILE* metaFile = fopen(metaDataPath, "wb");
            defer{ fclose(metaFile); };
            fwrite(&aligment, sizeof(ModelAligment), 1, metaFile);
            fwrite(pos, sizeof(MATH::vec3), aligment.numVerts, metaFile);
            fwrite(normal, sizeof(MATH::vec3), aligment.numNormals , metaFile);
            fwrite(uv, sizeof(MATH::vec2), aligment.numTextureCoords, metaFile);
            fwrite(index, sizeof(int), aligment.numIndexes, metaFile);
            block.currentIndex = 0;
            printf("Parsed :: %s with %d Vertexes and %d Indexes \n", currentName, aligment.numVerts, aligment.numIndexes);
#if 0
            for(int i2 = 0; i2 < aligment.numVerts;i2++)
            {
            	printf("verts %f , %f , %f \n",pos[i2].x, pos[i2].y, pos[i2].z);
           	    printf("norms %f , %f , %f \n", normal[i2].x, normal[i2].y, normal[i2].z);
            	printf("texs %f , %f \n", uv[i2].x, uv[i2].y);
            }  
#endif
    }



#endif


	return 0;
}
