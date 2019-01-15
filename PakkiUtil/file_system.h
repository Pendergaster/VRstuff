#ifndef PAKKI_FILE_SYSTEM
#define PAKKI_FILE_SYSTEM
#include "Utils.h"
#include <tuple>
#include "Containers.h"
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>

#if __linux__
#include <unistd.h>
#elif defined (_WIN32)
#include <windows.h>
#include <tchar.h>
#endif
namespace FILESYS
{

#if 1//__linux__
#define PAKKI_FILETIME time_t
	struct FileHandle
	{
		PAKKI_FILETIME	fileTime;
	};

	bool get_filehandle(char *path,FileHandle* fileHandle) {
		struct stat attr;
		stat(path, &attr);
		//printf("Last modified time: %s", ctime(&attr.st_mtime));
		//fileHandle.fileTime = attr.st_mode ==  attr.st_mtime;
		if (-1 == stat(path, &attr)){
			return false;
		}
		fileHandle->fileTime = attr.st_mtime;
		return true;
	}


	bool compare_file_times(FileHandle rhv,FileHandle lhv)
	{
		return difftime(rhv.fileTime,lhv.fileTime) == 0;
	}
#endif

	struct file_util
	{
		char* file;
		size_t size;
		file_util(char* f,int _size) : file(f) , size(_size){};
		~file_util(){free(file);};
	};
	bool does_file_exist(const char* path)
	{
#if __linux__
		return access( path, F_OK ) != -1;
#elif defined(_WIN32)

		DWORD dwAttrib = GetFileAttributes(path);
		return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
				!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#endif
	}
	static void* load_binary_file_to_block(char* path,
			CONTAINER::MemoryBlock* block,size_t* fileSize)
	{
		FILE* fp = fopen(path,"rb");
		if(fp == NULL ) return NULL;
		void* ptrToMem = CONTAINER::get_next_memory_block(*block);
		fseek(fp, 0, SEEK_END);
		size_t len = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		CONTAINER::increase_memory_block_aligned(block,(int)len);
		fread(ptrToMem,len,1,fp);
		if(fileSize) *fileSize = len;
		return ptrToMem;
	}
	static void* load_binary_file(const char* path,uint* size = NULL)
	{
		FILE* fp = fopen(path,"rb");
		if(fp == NULL ) return NULL;
		defer{fclose(fp);};
		//void* ptrToMem = CONTAINER::get_next_memory_block(*block);
		fseek(fp, 0, SEEK_END);
		size_t len = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		void* ptr = malloc(len);
		fread(ptr,len,1,fp);
		if(size)
		{
			*size = (uint)len;
		}
		return ptr;
	}

	char* load_file_and_free(const char* path,size_t* len)
	{
		char *source = NULL;
		FILE *fp = fopen(path, "r");
		defer {fclose(fp);};
		size_t newLen = 0;
		if (fp != NULL) {
			/* Go to the end of the file. */
			if (fseek(fp, 0L, SEEK_END) == 0) {
				/* Get the size of the file. */
				long bufsize = ftell(fp);
				if (bufsize == -1) { /* Error */ }

				/* Allocate our buffer to that size. */
				source = (char*)malloc(sizeof(char) * (bufsize + 1));

				/* Go back to the start of the file. */
				if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }

				/* Read the entire file into memory. */
				newLen = fread(source, sizeof(char), bufsize, fp);
				ASSERT_MESSAGE(ferror(fp) == 0 , "ERROR LOADING FILE \n");
				source[newLen++] = '\0'; /* Just to be safe. */
			}
		}
		else
		{
			ABORT_MESSAGE("FILE NOT FOUND ,%s \n",path);
		}
		*len = newLen;
		return source;
	}
	file_util load_file(const char* path)
	{
		size_t len = 0;	  
		char* data = load_file_and_free(path,&len);
		return file_util(data,(int)len);
	}

	//std::tuple<char*,size_t,size_t> 
	static inline void load_to_buffer(const char* path,char* buffer,
			size_t allocatedSize,size_t* currentSize)
	{
		FILE *fp = fopen(path, "r");
		ASSERT_MESSAGE(fp != NULL, "FILE NOT FOUND %s \n", path);
		defer {fclose(fp);};
		size_t newLen = 0;
		if(fp != NULL)
		{
			if (fseek(fp, 0L, SEEK_END) == 0) {
				/* Get the size of the file. */
				int bufsize = ftell(fp);
				ASSERT_MESSAGE(bufsize != -1,"ERROR READING FILE \n");

				if(allocatedSize - *currentSize < (size_t)bufsize)
				{
					ABORT_MESSAGE("MEMORY ERROR");
					return;
				}
				/* Go back to the start of the file. */
				if (fseek(fp, 0L, SEEK_SET) != 0)
				{ ABORT_MESSAGE("SOMETHING WENT WRONG"); }
				/* Read the entire file into memory. */
				newLen = fread((char*)((int8*)buffer + *currentSize) 
						, sizeof(char), bufsize, fp);
				ASSERT_MESSAGE(ferror(fp) == 0 , "ERROR LOADING FILE \n");
				buffer[newLen] = '\0';
			}
		}
		else
		{
			ABORT_MESSAGE("FILE NOT FOUND ,%s \n",path);
		}
		*currentSize = newLen;
	}
	static inline char* load_multiple_files_to_mem_block(
			const char** names, uint numFiles, 
			CONTAINER::MemoryBlock* block,size_t* size)
	{
		FILE* fp = NULL;
		char* startOfFiles = (char*)CONTAINER::get_next_memory_block(*block);
		int sizeOfAllShaders = 0;
		for(uint i = 0; i < numFiles; i++)	
		{
			const char* path = names[i];
			printf("parsing %s ----\n",path);
			fp = fopen(path,"r");
			//to end
			fseek(fp, 0L, SEEK_END);
			ASSERT_MESSAGE(fp != NULL,"FILE NOT FOUND %s \n",path);
			defer {fclose(fp);};
			int bufsize = ftell(fp);
			ASSERT_MESSAGE(bufsize != -1,"ERROR READING FILE \n");
			if (fseek(fp, 0L, SEEK_SET) != 0) 
			{ ABORT_MESSAGE("SOMETHING WENT WRONG"); }
			/* Read the entire file into memory. */
			size_t newLen = fread((char*)(
						((int8*)CONTAINER::get_next_memory_block(*block)) 
						+ sizeOfAllShaders)
					, sizeof(char), bufsize, fp);

			sizeOfAllShaders += (int)newLen;//bufsize;
			*size += newLen;
			ASSERT_MESSAGE(ferror(fp) == 0 , "ERROR LOADING FILE \n");
		}
		startOfFiles[sizeOfAllShaders] = '\0';
		CONTAINER::increase_memory_block_aligned(block,sizeOfAllShaders);
		return startOfFiles;
	}

	static inline char* load_file_to_memblock(const char* path,
			CONTAINER::MemoryBlock* block,size_t* size)
	{
		char* ret = (char*)CONTAINER::get_next_memory_block(*block);
		size_t tempCurrentSize = 0;//block->currentIndex;
		load_to_buffer(path,ret,(size_t)block->size,&tempCurrentSize);
		*size = tempCurrentSize + 1;//tempCurrentSize -block->currentIndex;
		ret[tempCurrentSize +1]  = '\0';
		CONTAINER::increase_memory_block_aligned(block,(int)tempCurrentSize + 1);//tempCurrentSize -block->currentIndex);
		//block->currentIndex = tempCurrentSize;
		return ret;
	}

}
#endif
