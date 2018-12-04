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
	CONTAINER::MemoryBlock before = *block;
	defer {*block = before;};
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
	bones.resize(aligment.numVerts);
	if(animated)
	{
		for(uint b = 0; b < mesh->mNumBones;b++)
		{
			aiBone* currentBones = mesh->mBones[b]; 
			uint numWeights = mesh->mBones[b]->mNumWeights;
			std::string boneName(mesh->mBones[b]->mName.data);
			int boneIndex = 0;
			if(bonemapping.find(boneName) == bonemapping.end()) // nothing found, create new
			{
				boneIndex = (uint)bonemapping.size();
				bonemapping[boneName] = boneIndex;
				BoneData bone;
				boneData.push_back(bone);
			}
			else
			{
				boneIndex = bonemapping[boneName];
			}
			bonemapping[boneName] = boneIndex;
			// bone to node space
			boneData[boneIndex].offset = *(MATH::mat4*)&mesh->mBones[b]->mOffsetMatrix;
			for(uint w = 0; w < numWeights;w++)
			{
				//float currentWeight = mesh->mBones[b]->mWeights[w].mWeight;
				uint vertexId = mesh->mBones[b]->mWeights[w].mVertexId;
					bones[vertexId].add(boneIndex,mesh->mBones[b]->mWeights[w].mWeight);
			}
		}
		#if 0
		for(uint i = 0; i < bones.size(); i++)
		{
			float add = 0;
			for(int p = 0;p < MAX_BONES_IN_VERTEX; p++){
				add += bones[i].weights[p];
			}
			add = sqrtf(add);
			ASSERT_MESSAGE(add != 0,"WEIGHTS ARE ZERO!");
			//add /= (float)MAX_BONES_IN_VERTEX;
			for(int p = 0;p < MAX_BONES_IN_VERTEX; p++){
				bones[i].weights[p] /= add;
			}
		}
		#endif
		aligment.numBoneData = (uint)bones.size();
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

uint numpasses = 0;
void push_node(aiNode* node, std::vector<RenderNode>& nodes,
		std::map<std::string,uint>& nodeMapping,std::map<std::string,uint>& boneMapping)
{
	if(numpasses == 20){
		int kkkk = 0;
	}
	RenderNode n;
	n.numChildren = node->mNumChildren;
	ASSERT_MESSAGE(node->mNumMeshes <= 1,"MORE THAN TWO BODIES IN NODE");
	if(node->mNumMeshes){
		n.meshIndex = node->mMeshes[0];
	} else {
		n.meshIndex = NO_MESH;
	}
	std::string name(node->mName.data);

	if(boneMapping.find(name) == boneMapping.end()) {
		n.boneIndex = NO_BONE;
	} else {
		n.boneIndex = boneMapping[name];
	}

	n.numChildren = node->mNumChildren;
    n.transformation = (*((MATH::mat4*)&node->mTransformation));
	if(nodeMapping.find(name) == nodeMapping.end()) {
		nodeMapping[name] = (uint)nodes.size();
	} else {
		ABORT_MESSAGE("node is already found!");
	}
	nodes.push_back(n);
	numpasses++;
	for(uint i = 0; i < node->mNumChildren;i++)
	{
		push_node(node->mChildren[i],nodes,nodeMapping,boneMapping);
	}
}

std::string load_animation(aiAnimation* anim,char* metaFileName,
			std::map<std::string,uint>& nodeMapping,uint numAnim,
			uint* allNumChannels,uint* allNumPositions,uint* allNumRotations,uint* allNumScales)
{
	AnimationData data;
	data.duration = anim->mDuration;
	data.ticksPerSecond = anim->mTicksPerSecond;
	data.numChannels = anim->mNumChannels;


	std::string path(metaFileName);
	path += anim->mName.data + std::string(".anim") + std::to_string(numAnim);
	FILE* animFile = fopen(path.data(),"wb");
	defer {fclose(animFile);};
	fwrite(&data,sizeof(AnimationData),1,animFile);
	*allNumChannels += data.numChannels;
	for(uint i = 0 ; i < data.numChannels; i++)
	{
		aiNodeAnim* node = anim->mChannels[i];
		AnimationChannel channels;
		channels.numPositionKeys = node->mNumPositionKeys;
		channels.numRotationKeys = node->mNumRotationKeys;
		channels.numScaleKeys = node->mNumScalingKeys;
		std::string affectedNode(node->mNodeName.data);
		if(nodeMapping.find(affectedNode) == nodeMapping.end()) {
			ABORT_MESSAGE("AFFECTED NODE NOT FOUND!");
		} else {
			channels.nodeIndex = nodeMapping[affectedNode];
		}
		std::vector<RotationKey> rotationKeys;
		std::vector<PositionKey> positionKeys;
		std::vector<ScaleKey> scalingKeys;
		for(uint j = 0; j < channels.numPositionKeys; j++)
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
		for(uint j = 0; j < channels.numRotationKeys; j++)
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
		for(uint j = 0; j < channels.numScaleKeys; j++)
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
		*allNumPositions += channels.numPositionKeys;
		*allNumRotations += channels.numRotationKeys;
		*allNumScales += channels.numScaleKeys;
		fwrite(&channels,sizeof(AnimationChannel),1,animFile);
		fwrite(positionKeys.data(),sizeof(PositionKey),positionKeys.size(),animFile);
		fwrite(rotationKeys.data(),sizeof(RotationKey),rotationKeys.size(),animFile);
		fwrite(scalingKeys.data(),sizeof(ScaleKey),scalingKeys.size(),animFile);
	}

	printf("loaded animation %s with %d channels \n", anim->mName.data,data.numChannels);
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
	uint allMeshes = 0;
	uint allNodes = 0;
	uint allChannels = 0;
	uint allRotationKeys = 0;
	uint allPositionKeys = 0;
	uint allScaleKeys = 0;
	uint allAnimations = 0;
	uint allBones = 0;
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
		ASSERT_MESSAGE(path && metaPath && meshdatapath,"RAWPATH IS NOT FOUND ::  %s \n",currentName);
		const aiScene* scene = importer.ReadFile(path,0);
				//aiProcess_Triangulate | aiProcess_FlipUVs |
				//aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_GenUVCoords );
		ASSERT_MESSAGE(scene && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mRootNode, "FAILED TO PARSE :: %s ERROR :: %s \n", currentName, importer.GetErrorString());
		int maxMeshes = scene->mNumMeshes;
				printf("Parsing model %d/%d :: %s \n Num meshes %d \n", i, modelNames.numobj, currentName, maxMeshes);
		bool animated = (*currentToken)["Animated"].GetBool();
		//std::vector<BoneData> boneData;
		std::map<std::string,uint> boneMapping;
		allMeshes += scene->mNumMeshes;
		for(uint meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
		{
			std::string name = load_mesh(
					scene->mMeshes[meshIndex],
					&block,meshIndex,
					meshdatapath,boneMapping,animated,bones);	
			meshNames.push_back(name);
		}
		allBones += boneMapping.size();

		//safe tree to data structure
		FILE* infoFile = fopen(metaPath,"w");
		defer {fclose(infoFile);};
		ASSERT_MESSAGE(infoFile ,"INFO FILE IS NOT CREATED ::  %s \n",currentName);
		fprintf(infoFile, "%d \n", scene->mNumMeshes);
		for(uint meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
		{
			fprintf(infoFile, "%s \n", meshNames[meshIndex].data()); // frite vertex data files to info
		}
		std::vector<RenderNode> renderNodes;
		std::map<std::string,uint> nodeMapping;
		push_node(scene->mRootNode,renderNodes,nodeMapping,boneMapping);
		allNodes += (uint)renderNodes.size();
		std::string treePath(meshdatapath);
		treePath += ".nodes";
		FILE* treeFile = fopen(treePath.data(),"wb");
		defer {fclose(treeFile);};
		MATH::mat4 globalTransform = *(MATH::mat4*)&scene->mRootNode->mTransformation;
		MATH::mat4 inv;
		MATH::inverse_mat4(&inv, &globalTransform);
		fwrite(&inv,sizeof(MATH::mat4), 1,treeFile);
		fwrite(renderNodes.data(),sizeof(RenderNode),
				renderNodes.size(),treeFile);
		fprintf(infoFile, "%d \n", (uint)renderNodes.size()); // frite vertex data files to info
		fprintf(infoFile, "%s \n", treePath.data()); // frite vertex data files to info
		//FILE* 
		// write bones 
		std::string boneFilePath = std::string(metaPath) + std::string(".bone"); 
		fprintf(infoFile, "%d \n", bones.size()); // write bone file path
		fprintf(infoFile, "%s \n", boneFilePath.data()); // write bone file path
		FILE* boneFile = fopen(boneFilePath.c_str(),"wb");
		defer{fclose(boneFile);};
		fwrite(bones.data(),sizeof(BoneData),bones.size(),boneFile);

		ASSERT_MESSAGE(scene->mNumAnimations,"NO ANIMATIONS");
		fprintf(infoFile, "%s \n", "1"); // frite vertex data files to info
		std::string animPath = load_animation(scene->mAnimations[0]
					,metaPath,nodeMapping,0,&allChannels,&allPositionKeys,&allRotationKeys,&allScaleKeys);
		fprintf(infoFile, "%s \n", animPath.data()); // frite vertex data files to info
		allAnimations += 1;		
#if 0
		std::vector<MeshPart> meshParts;
		while(!nodes.empty()) // 
		{
			Node current = nodes.back();
			int numNodesLoopped = 0;
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

#endif
	}
	FILE* hostfile =  fopen("temp/root.info","w");

	fprintf(hostfile,  "{\n\t\"NumModels\" : %d, \n\t\"NumMeshes\" : %d, \n\t\"NumAnimations\" : %d, \n\t\"NumAnimationChannels\" : %d, \n\t\"NumNodes\" : %d, \n\t\"NumBones\" : %d \n\t\"NumPositionKeys\" : %d, \n\t\"NumRotationKeys\" : %d, \n\t\"NumScaleKeys\" : %d \n}",
					modelNames.numobj,allMeshes,allAnimations,allChannels,allNodes,allBones,allPositionKeys,allRotationKeys,allScaleKeys);
	fclose(hostfile);
	printf("bye \n");
	// frite vertex data files to info
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
