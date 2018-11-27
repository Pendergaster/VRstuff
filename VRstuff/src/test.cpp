#define _CRT_SECURE_NO_WARNINGS
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
#include <string>
#include <vector>
#include <map>
#include <array>

struct objIndex
{
	int vertIndex = 0;
	int normIndex = 0;
	int texIndex = 0;
};
#undef calloc
#if 0
std::string load_bones(aiMesh* mesh,char* name,int numMesh,std::map<std::string,uint>& bonemapping ,
		uint* numBones,std::vector<BoneData>& boneInfos)
{
	std::vector<VertexBoneData> bones;
	ASSERT_MESSAGE(mesh->mNumBones < MAX_BONES,"TOO MANY BONES");
	bones.resize(mesh->mNumVertices);
	std::string ret(name);
	ret += std::to_string(numMesh);
	ret += ".anim";
	uint numVertsWrote = 0;
	for(uint b = 0; b < mesh->mNumBones;b++)
	{
		uint numWeights = mesh->mBones[b]->mNumWeights;
		std::string boneName(mesh->mBones[b]->mName.data);
		int boneIndex = 0;
		if(bonemapping.find(boneName) == bonemapping.end()) // nothing found, create new
		{
			boneIndex = *numBones;
			BoneData boneInfo;
			boneInfo.offset = *(MATH::mat4*)&mesh->mBones[b]->mOffsetMatrix;
			MATH::mat4 idenMat;
			MATH::identify(&idenMat);
			boneInfo.offset = idenMat;
			boneInfos.push_back(boneInfo);
			bonemapping[boneName] = *numBones;
			numBones++;
		}
		else
		{
			assert(0);
			boneIndex = bonemapping[boneName];
		}
		for(uint w = 0; w < numWeights;w++)
		{
			//float currentWeight = mesh->mBones[b]->mWeights[w].mWeight;
			uint vertexId = mesh->mBones[b]->mWeights[w].mVertexId;
			bones[vertexId].add(boneIndex,mesh->mBones[b]->mWeights[w].mWeight);
			numVertsWrote++;
		}
	}
	for(uint i = 0; i < bones.size(); i++)
	{
		float add = 0;
		for(int p = 0;p < MAX_BONES_IN_VERTEX; p++){
			add += bones[i].weights[p];
		}
		ASSERT_MESSAGE(add != 0,"WEIGHTS ARE ZERO!");
		add /= (float)MAX_BONES_IN_VERTEX;
		for(int p = 0;p < MAX_BONES_IN_VERTEX; p++){
			bones[i].weights[p] /= add;
		}
	}

	FILE* skinFile = fopen(ret.c_str(),"wb");
	uint numskins = ((uint)bones.size());
	fwrite(&numskins, sizeof(uint),1,skinFile);
	fwrite(bones.data(), sizeof(VertexBoneData),bones.size(),skinFile);
	printf("Animation %s wrote with %d verts \n",ret.c_str(),numVertsWrote);
	return ret;
	//bonetransforms.resize(numBones);
}
#endif

std::string load_mesh(aiMesh* mesh,CONTAINER::MemoryBlock* block,uint numMesh,char* path,
		std::map<std::string,uint>& bonemapping,bool animated,
		std::vector<BoneData>& boneData)
{	
	MATH::vec3* pos = (MATH::vec3*)CONTAINER::get_next_memory_block(*block);
	CONTAINER::increase_memory_block(block, mesh->mNumVertices * sizeof(MATH::vec3));
	MATH::vec3* normal = (MATH::vec3*)CONTAINER::get_next_memory_block(*block);
	CONTAINER::increase_memory_block(block, mesh->mNumVertices * sizeof(MATH::vec3));
	MATH::vec2* uv = (MATH::vec2*)CONTAINER::get_next_memory_block(*block);

	CONTAINER::increase_memory_block(block, mesh->mNumVertices * sizeof(MATH::vec2));
	int* index = (int*)CONTAINER::get_next_memory_block(*block);

	ASSERT_MESSAGE(mesh->mTextureCoords[0],"NO TEXTURE COORDINATES IN :: %s \n",path);

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
	uint indexIndex = 0;
	std::string modelName;
	for (unsigned int i2 = 0; i2 < mesh->mNumFaces; i2++)
	{
		aiFace face = mesh->mFaces[i2];
		for (unsigned int i3 = 0; i3 < face.mNumIndices; i3++)
		{
			CONTAINER::ensure_memory_block(block, indexIndex * sizeof(int));
			index[indexIndex++] = face.mIndices[i3];
		}
	}

	modelName = path + std::to_string(numMesh);

	meshAligment aligment;
	aligment.numVerts = mesh->mNumVertices;
	aligment.numNormals = mesh->mNumVertices;
	aligment.numTextureCoords = mesh->mNumVertices;
	aligment.numIndexes = indexIndex;

	std::vector<VertexBoneData> bones;
	if(animated)
	{
		for(uint b = 0; b < mesh->mNumBones;b++)
		{
			uint numWeights = mesh->mBones[b]->mNumWeights;
			std::string boneName(mesh->mBones[b]->mName.data);
			int boneIndex = 0;
			if(bonemapping.find(boneName) == bonemapping.end()) // nothing found, create new
			{
				boneIndex = bonemapping.size();
				bonemapping[boneName] = boneIndex;
				BoneData bone;
				boneData.push_back(bone);
			}
			else
			{
				boneIndex = bonemapping[boneName];
			}
			bonemapping[boneName] = boneIndex;
			boneData[boneIndex].offset = *(MATH::mat4*)&mesh->mBones[b]->mOffsetMatrix;
			for(uint w = 0; w < numWeights;w++)
			{
				//float currentWeight = mesh->mBones[b]->mWeights[w].mWeight;
				uint vertexId = mesh->mBones[b]->mWeights[w].mVertexId;
				bones[vertexId].add(boneIndex,mesh->mBones[b]->mWeights[w].mWeight);
			}
		}
		for(uint i = 0; i < bones.size(); i++)
		{
			float add = 0;
			for(int p = 0;p < MAX_BONES_IN_VERTEX; p++){
				add += bones[i].weights[p];
			}
			ASSERT_MESSAGE(add != 0,"WEIGHTS ARE ZERO!");
			add /= (float)MAX_BONES_IN_VERTEX;
			for(int p = 0;p < MAX_BONES_IN_VERTEX; p++){
				bones[i].weights[p] /= add;
			}
		}
		aligment.numBoneData = bones.size();
	}
	FILE* metaFile = fopen(modelName.data(), "wb");
	defer{ fclose(metaFile); };
	fwrite(&aligment, sizeof(meshAligment), 1, metaFile);
	fwrite(pos, sizeof(MATH::vec3), aligment.numVerts, metaFile);
	fwrite(normal, sizeof(MATH::vec3), aligment.numNormals , metaFile);
	fwrite(uv, sizeof(MATH::vec2), aligment.numTextureCoords, metaFile);
	fwrite(index, sizeof(int), aligment.numIndexes, metaFile);
	if(animated){
		fwrite(bones.data(), sizeof(VertexBoneData),bones.size(),metaFile);
	}

	block->currentIndex = 0;
	printf("Parsed :: %s file %s with %d Vertexes and %d Indexes %d bones \n", path ,modelName.data() , 
			aligment.numVerts, aligment.numIndexes,aligment.numBoneData);

#if 0
	for(int i2 = 0; i2 < aligment.numVerts;i2++)
	{
		printf("verts %f , %f , %f \n",pos[i2].x, pos[i2].y, pos[i2].z);
		printf("norms %f , %f , %f \n", normal[i2].x, normal[i2].y, normal[i2].z);
		printf("texs %f , %f \n", uv[i2].x, uv[i2].y);
	}  
#endif	
	return modelName;
}

struct Node
{
	Node(){};
	Node(MATH::mat4 _matrix,aiNode*	_assimpNode): matrix (_matrix), assimpNode (_assimpNode){}
	MATH::mat4 matrix;
	aiNode*	   assimpNode;
};

std::string load_animation(aiAnimation* anim,uint numAnim,char* metaFileName)
{
	AnimationData data;
	data.duration = anim->mDuration;
	data.ticksPerSecond = anim->mTicksPerSecond;
	data.numChannels = anim->mNumChannels;
	std::string path(metaFileName);
	path += anim->mName.data + std::string(".anim");
	FILE* animFile = fopen(path.data(),"wb");
	defer {fclose(animFile);};
	fwrite(&data,sizeof(AnimationData),1,animFile);
	

	for(uint i = 0 ; i < data.numChannels; i++)
	{
		aiNodeAnim* node = anim->mChannels[i];
		AnimationChannel channels;
		channels.numPositionKeys = node->mNumPositionKeys;
		channels.numRotationKeys = node->mNumRotationKeys;
		channels.numScaleKeys = node->mNumScalingKeys;
		fwrite(&channels,sizeof(AnimationChannel),1,animFile);
		std::vector<RotationKey> rotationKeys;
		std::vector<PositionKey> positionKeys;
		std::vector<ScaleKey> scalingKeys;
		for(int j = 0; j < channels.numPositionKeys; j++)
		{
			PositionKey temp;
			temp.position = MATH::vec3(
				node->mPositionKeys[j].mValue.x,
				node->mPositionKeys[j].mValue.y,
				node->mPositionKeys[j].mValue.z
			);
			temp.time = node->mPositionKeys[j].mTime;
			positionKeys.push_back(temp);
		}
		for(int j = 0; j < channels.numRotationKeys; j++)
		{
			RotationKey temp;
			temp.quat = MATH::quaternion(
				node->mRotationKeys[j].mValue.w,
				node->mRotationKeys[j].mValue.x,
				node->mRotationKeys[j].mValue.y,
				node->mRotationKeys[j].mValue.z
			);
			temp.time = node->mRotationKeys[j].mTime;
			rotationKeys.push_back(temp);
		}
		for(int j = 0; j < channels.numScaleKeys; j++)
		{
			ScaleKey temp;
			temp.scale = MATH::vec3(
				node->mScalingKeys[j].mValue.x,
				node->mScalingKeys[j].mValue.y,
				node->mScalingKeys[j].mValue.z
			);
			temp.time = node->mScalingKeys[j].mTime;
			scalingKeys.push_back(temp);
		}
		fwrite(positionKeys.data(),sizeof(PositionKey),positionKeys.size(),animFile);
		fwrite(rotationKeys.data(),sizeof(RotationKey),rotationKeys.size(),animFile);
		fwrite(scalingKeys.data(),sizeof(ScaleKey),scalingKeys.size(),animFile);
	}

	printf("loaded animation %s with %d channels", anim->mName.data,data.numChannels);
	return path;
}

int main()
{
	JsonToken modelToken;
	modelToken.ParseFile("importdata/meshdata.json");
	CONTAINER::DynamicArray<char*> modelNames;
	CONTAINER::init_dynamic_array(&modelNames);
	defer{ CONTAINER::dispose_dynamic_array(&modelNames); };;
	modelToken.GetKeys(&modelNames);

	Assimp::Importer importer;
	CONTAINER::MemoryBlock block;
	CONTAINER::init_memory_block(&block, 6000000);
	MATH::mat4 identity;
	MATH::identify(&identity);
	for (uint i = 0; i < modelNames.numobj; i++)
	{		
		std::vector<BoneData> bones;
		std::vector<std::string> meshNames;
		std::vector<std::string> skinFileNames;
		char* currentName = modelNames.buffer[i];

		JsonToken* currentToken = modelToken[currentName].GetToken();
		ASSERT_MESSAGE(currentToken,"MODELDATA IS NOT VALID ::  %s \n",currentName);
		char* path = (*currentToken)["rawPath"].GetString();
		char* metaPath = (*currentToken)["metaDataPath"].GetString();
		char* meshdatapath = (*currentToken)["meshDataName"].GetString();
		char* meshpartpath = (*currentToken)["meshPartFile"].GetString();
		ASSERT_MESSAGE(path && metaPath && meshdatapath && meshpartpath,"RAWPATH IS NOT FOUND ::  %s \n",currentName);
		const aiScene* scene = importer.ReadFile(path,aiProcess_Triangulate | aiProcess_FlipUVs |
				aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_GenUVCoords );
		//aiProcess_GenUVCoords 
		//aiProcess_GenNormals 
		//const char * error = importer.GetErrorString();
		ASSERT_MESSAGE(scene && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mRootNode, "FAILED TO PARSE :: %s ERROR :: %s \n", currentName, importer.GetErrorString());
		int maxMeshes = scene->mNumMeshes;
		printf("Parsing model %d/%d :: %s \n Num meshes %d \n", i, modelNames.numobj, currentName, maxMeshes);
		std::vector<Node> nodes;

		nodes.emplace_back(identity,scene->mRootNode);
		bool animated = (*currentToken)["Animated"].GetBool();
		std::map<std::string,uint> boneMapping;
		std::map<std::string,uint> nodeMapping;
		std::vector<BoneData> boneData;
		uint numBones = 0;
		//safe tree to data structure
		for(uint meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
		{
#if 0
			if(animated)
			{

				//std::string load_bones(aiMesh* mesh,char* name,int numMesh,std::map<std::string,uint>& bonemapping ,uint numBones,std::vector<BoneInfo>& boneInfos)
				std::string name = load_bones(scene->mMeshes[meshIndex],meshdatapath,meshIndex,boneMapping,
						&numBones,boneData,);
				skinFileNames.push_back(name);
			}
#endif
			std::string name = load_mesh(
					scene->mMeshes[meshIndex],
					&block,meshIndex,
					meshdatapath,boneMapping,animated,bones);	
			meshNames.push_back(name);
		}
		FILE* infoFile = fopen(metaPath,"w");
		defer {fclose(infoFile);};
		ASSERT_MESSAGE(infoFile ,"INFO FILE IS NOT CREATED ::  %s \n",currentName);
		fprintf(infoFile, "%d \n", scene->mNumMeshes);
		for(uint meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
		{
			fprintf(infoFile, "%s \n", meshNames[meshIndex].data()); // frite vertex data files to info
#if 0
			if(animated){
				fprintf(infoFile, "%s \n", skinFileNames[meshIndex].data()); // frite skinning info 
			}
#endif
		}
		fprintf(infoFile, "%s \n", meshpartpath);					//where parts of the mesh is
		FILE* partFile = fopen(meshpartpath,"wb");
		defer {fclose(partFile);};
		std::vector<MeshPart> meshParts;
		while(!nodes.empty()) // 
		{
			Node current = nodes.back();
			nodes.pop_back();
			for(int childIndex = 0;childIndex < (int)current.assimpNode->mNumChildren; childIndex++)
			{
				MATH::mat4 mTemp = current.matrix * (*((MATH::mat4*)&current.assimpNode->mChildren[childIndex]->mTransformation));
				nodes.push_back
					({
					 mTemp,
					 current.assimpNode->mChildren[childIndex]
					 });
			}
			//write current mesh to parts list and file for it			
			for(int meshIndex = 0; meshIndex < (int)current.assimpNode->mNumMeshes; meshIndex++) // for every 
			{
				MeshPart currentPart;
				currentPart.localTranform = current.matrix;
				currentPart.meshIndex = current.assimpNode->mMeshes[meshIndex];
				meshParts.push_back(currentPart);
			}			
		}

		int numParts = meshParts.size();
		fwrite(&numParts, sizeof(int), 1, partFile);
		//meshdata
		for(const MeshPart& mp : meshParts)
		{
			fwrite(&mp, sizeof(MeshPart), 1, partFile);
		}			

	}
	return 0;
}








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
