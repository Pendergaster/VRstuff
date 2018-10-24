#ifndef PAKKI_TEXTURES
#define PAKKI_TEXTURES
#include <Containers.h>
#include <JsonToken.h>
#include <Utils.h>
#include<stb_image.h>
#include <glad/glad.h>
#include <texturedefs.h>
#include "glerrorcheck.h"
static inline bool load_texture(TextureInfo* info, uint id)
{
    //int widht = 0 ,height = 0 ,channels = 0;
    TextureInfo temp = *info;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLenum textype = 0;
    if (info->texType == TextureType::Texture2D)
    {
        textype = GL_TEXTURE_2D;
    }
    else if (info->texType == TextureType::CubeMap)
    {
        textype = GL_TEXTURE_CUBE_MAP;
    }

    glBindTexture(textype, id);
    GLenum mode3 = 0;// temp.channels == 3 ? GL_RGB : GL_RGBA;
    GLenum mode2 = 0;// temp.srgb ? (temp.channels == 3 ? GL_SRGB : GL_SRGB_ALPHA) : mode3;
    if (info->texType == TextureType::CubeMap)
    {
        temp.widht = 0;
        temp.height = 0;
        temp.channels = 0;
        int width, height;
        printf("CUBE MAP ID %d \n",id);
        for (unsigned int i = 0; i < 6; i++)
        {
            unsigned char *data = stbi_load(temp.frontPath, &width, &height, &temp.channels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                );
                stbi_image_free(data);
            }
            else
            {
                stbi_image_free(data);
                ABORT_MESSAGE("TEXTURE NOT FOUND %s \n", temp.path);
            }
        }
    }
    else
    {
        unsigned char* data = stbi_load(temp.path, &temp.widht, &temp.height, &temp.channels, 0);
        mode3 =  temp.channels == 3 ? GL_RGB : GL_RGBA;
        mode2 =  temp.srgb ? (temp.channels == 3 ? GL_SRGB : GL_SRGB_ALPHA) : mode3;
        //ASSERT_MESSAGE(data,"FAILED TO LOAD TEXTURE :: %s \n",temp.name);
        if (!data) {
            LOG("Texture not found %s \n", temp.path);
            return false;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, mode2, temp.widht, temp.height, 0, mode3, GL_UNSIGNED_BYTE, data);
        defer{ stbi_image_free(data); };
    }



    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,temp.wrapMode);
    GLenum mode = 0;
    if (temp.wrapMode == WrapMode::ClampToBorder) {
        mode = GL_CLAMP_TO_BORDER;
    }
    else if (temp.wrapMode == WrapMode::ClampToEdge) {
        mode = GL_CLAMP_TO_EDGE;
    }
    else if (temp.wrapMode == WrapMode::MirroredRepeat) {
        mode = GL_MIRRORED_REPEAT;
    }
    else if (temp.wrapMode == WrapMode::Repeat) {
        mode = GL_REPEAT;
    }

    glTexParameteri(textype, GL_TEXTURE_WRAP_S, mode);
    glTexParameteri(textype, GL_TEXTURE_WRAP_T, mode);
    if (textype == GL_TEXTURE_CUBE_MAP) {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    glTexParameterf(textype, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(textype, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


  


    if (temp.mipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    *info = temp;
    return true;
}
//TODO define stb malloc ja realloc ja free
//TODO testaa json true ja false
//TODO siivoo kommentit
static void init_textures_from_metadata(TextureData* textureData,
    CONTAINER::MemoryBlock* staticMem)
    //JsonToken* token,uint* textureIds,TextureInfo* textureInfos,int* numTextures,
    //CONTAINER::StringTable<int>* textureTable,CONTAINER::DynamicArray<char*> names,
    //CONTAINER::MemoryBlock* staticMem)
{
    JsonToken token;
    token.ParseFile("importdata/texturedata.json");
    CONTAINER::DynamicArray<char*> names;
    CONTAINER::init_dynamic_array(&names);
    defer{ CONTAINER::dispose_dynamic_array(&names); };
    token.GetKeys(&names);

    textureData->textureInfos = (TextureInfo*)CONTAINER::get_next_memory_block(*staticMem);
    CONTAINER::increase_memory_block(staticMem, names.numobj * sizeof(TextureInfo));
    textureData->textureIds = (uint*)CONTAINER::get_next_memory_block(*staticMem);
    CONTAINER::increase_memory_block(staticMem, names.numobj * sizeof(uint));

    glGenTextures(names.numobj, textureData->textureIds);
    textureData->numTextures = names.numobj;
    CONTAINER::init_table_with_block<int>(&textureData->textureCache, staticMem,
        (int)textureData->numTextures);
    for (uint i = 0; i < names.numobj; i++)
    {
        TextureInfo info;
        char* currentName = names.buffer[i];
        JsonToken* currentToken = token[currentName].GetToken();
        ASSERT_MESSAGE(currentToken, "TEXTURE DATA IS NOT VALID :: %s \n", currentName);
        //info.channels = channels;
        //info.height = height;
        //info.widht = widht;
        info.name = (char*)CONTAINER::get_next_memory_block(*staticMem);
        strcpy(info.name, currentName);
        CONTAINER::increase_memory_block_aligned(staticMem, (int)strlen(info.name) + 1);
        JValueHandle val = (*currentToken)["Mipmap"];
        ASSERT_MESSAGE(val.IsValid(), "MIPMAP IS NOT DEFINED :: %s \n", currentName);
        info.mipmap = val.GetBool();
        val = (*currentToken)["SRGB"];
        ASSERT_MESSAGE(val.IsValid(), "MIPMAP IS NOT DEFINED :: %s \n", currentName);
        info.srgb = val.GetBool();

        char* wrapMode = (*currentToken)["Wrap"].GetString();
        ASSERT_MESSAGE(wrapMode, "WRAP MODE NOT DEFINED FOR %s", info.name);
        int i2 = 0;
        for (; i2 < WrapMode::MaxModes; i2++)
        {
            if (!strcmp(wrapMode, WRAP_MODE_NAMES[i2]))
            {
                info.wrapMode = i2;
                break;
            }
        }
        ASSERT_MESSAGE(i2 < WrapMode::MaxModes, "WRAPMODE NOT DEFINED FOR %s", info.name);
        char* textype = (*currentToken)["TextureType"].GetString();
        ASSERT_MESSAGE(textype, "TEX TYPE NOT DEFINED FOR %s", info.name);
        i2 = 0;
        for (; i2 < TextureType::MaxTexTypes; i2++)
        {
            if (!strcmp(textype, TEXTURE_TYPE_NAMES[i2]))
            {
                info.texType = i2;
                break;
            }
        }
        ASSERT_MESSAGE(i2 < TextureType::MaxTexTypes, "TEXTURETYPE NOT DEFINED FOR %s", info.name);
        if (info.texType != TextureType::CubeMap)
        {
            char* path = (*currentToken)["Path"].GetString();
            ASSERT_MESSAGE(path, "TEXTURE PATH IS NOT VALID :: %s \n", currentName);
            info.path = (char*)CONTAINER::get_next_memory_block(*staticMem);
            strcpy(info.path, path);
            CONTAINER::increase_memory_block_aligned(staticMem, (int)strlen(info.path) + 1);
        }
        else //cubemap
        {
            char* tempName = NULL;
            tempName = (*currentToken)["SkyFront"].GetString();
            ASSERT_MESSAGE(tempName, "TEXTURE PATH IS NOT VALID :: %s \n", currentName);
            info.frontPath = (char*)CONTAINER::get_next_memory_block(*staticMem);
            strcpy(info.frontPath, tempName);
            CONTAINER::increase_memory_block_aligned(staticMem, (int)strlen(info.path) + 1);

            tempName = (*currentToken)["SkyBack"].GetString();
            ASSERT_MESSAGE(tempName, "TEXTURE PATH IS NOT VALID :: %s \n", currentName);
            info.backPath = (char*)CONTAINER::get_next_memory_block(*staticMem);
            strcpy(info.backPath, tempName);
            CONTAINER::increase_memory_block_aligned(staticMem, (int)strlen(info.path) + 1);


            tempName = (*currentToken)["SkyUp"].GetString();
            ASSERT_MESSAGE(tempName, "TEXTURE PATH IS NOT VALID :: %s \n", currentName);
            info.topPath = (char*)CONTAINER::get_next_memory_block(*staticMem);
            strcpy(info.topPath, tempName);
            CONTAINER::increase_memory_block_aligned(staticMem, (int)strlen(info.path) + 1);


            tempName = (*currentToken)["SkyDown"].GetString();
            ASSERT_MESSAGE(tempName, "TEXTURE PATH IS NOT VALID :: %s \n", currentName);
            info.downPath = (char*)CONTAINER::get_next_memory_block(*staticMem);
            strcpy(info.downPath, tempName);
            CONTAINER::increase_memory_block_aligned(staticMem, (int)strlen(info.path) + 1);


            tempName = (*currentToken)["SkyRight"].GetString();
            ASSERT_MESSAGE(tempName, "TEXTURE PATH IS NOT VALID :: %s \n", currentName);
            info.rightPath = (char*)CONTAINER::get_next_memory_block(*staticMem);
            strcpy(info.rightPath, tempName);
            CONTAINER::increase_memory_block_aligned(staticMem, (int)strlen(info.path) + 1);


            tempName = (*currentToken)["SkyLeft"].GetString();
            ASSERT_MESSAGE(tempName, "TEXTURE PATH IS NOT VALID :: %s \n", currentName);
            info.leftPath = (char*)CONTAINER::get_next_memory_block(*staticMem);
            strcpy(info.leftPath, tempName);
            CONTAINER::increase_memory_block_aligned(staticMem, (int)strlen(info.path) + 1);

        }
        if (!load_texture(&info, textureData->textureIds[i]))
        {
            ABORT_MESSAGE("FAILED TO LOAD TEXTURE %s \n", currentName);
        }

        //TextureTypes 
        //TODO  lataa opngl puoli
        CONTAINER::insert_to_table<int>(&textureData->textureCache, info.name, i);
        textureData->textureInfos[i] = info;
    }
}




#endif
