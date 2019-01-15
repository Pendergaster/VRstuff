#ifndef CONTAINERS_H
#define CONTAINERS_H
#include <malloc.h>
#include <stdint.h>
#include <assert.h>
#include "Utils.h"
#include <string.h>
#include <math.h>

namespace CONTAINER
{
	typedef uint8_t _byte;

	template <typename T>
		struct DynamicArray
		{
			T* buffer 					= NULL;
			unsigned int numobj			= 0;
			unsigned int allocatedsize	= 0;
		};

	template <typename T>
		void 
		init_dynamic_array(DynamicArray<T>* array,unsigned int num = 10)
		{
			array->buffer = (T*)malloc(sizeof(T)*num);	
			if(!array->buffer) return;
			array->allocatedsize = num;
		}
	template <typename T>
		void
		ensure_size_dynamic_array(DynamicArray<T>* array,unsigned int numobj)
		{
			if(array->numobj+numobj >= array->allocatedsize)	
			{
				T* temp = array->buffer;	
				unsigned int newSize = array->numobj + numobj >= array->allocatedsize * 2 ? array->numobj + numobj + 10 : array->allocatedsize * 2;
				array->buffer = (T*)realloc(array->buffer,sizeof(T) *  newSize);
				if(!array->buffer)
				{
					array->buffer = temp;	
					return;
				}
				array->allocatedsize = newSize;	
			}
		}	
	template <typename T>
		inline void 
		push_back_dynamic_array(DynamicArray<T>* array,const T& obj)
		{

			ensure_size_dynamic_array<T>(array,1);
			array->buffer[array->numobj++] = obj;
		}
	template <typename T>
		inline void 
		clear_dynamic_array(DynamicArray<T>* array)
		{
			array->numobj= 0;
		}
	template <typename T>
		inline void 
		fast_delete_index_dynamic_array(DynamicArray<T>* array,unsigned int index)
		{
			array->numobj--;
			array->buffer[index] = array->buffer[array->numobj];
		}
	template <typename T>
		void
		dispose_dynamic_array(DynamicArray<T>* array)
		{
			if(array->buffer != NULL)
			{
				free(array->buffer);
				array->buffer = NULL;
			}

		}

#define TABLE_LENGHT 17 //keep this as prime number
#define TABLE_OVERFLOW_SIZE 20
	constexpr int TABLE_MAX_SIZE = TABLE_LENGHT + TABLE_OVERFLOW_SIZE;
	constexpr int DEFAULT_STRING_SIZE = 13 * sizeof(char);
#define HASH_CODE 151  // prime number also

	template<typename T>
		struct Pair
		{
			int 		key = 0;
			T 	  		val;	
			int			link = 0;
		};
	//TODO kato onko alku jos on ekoissa paikoissa
	//typedef unsigned int TableIterator;
	//#define TABLE_START 121212
	struct MemoryBlock
	{
		uint  	size = 0;
		uint 	currentIndex = 0;
		void* 	start = NULL;	
	};
	static inline void __ensure_mem_block_size__(MemoryBlock* block,uint add
			,const char* file,int line)
	{
		ASSERT_MESSAGE(add < (block->size - block->currentIndex),
				"MEMORY DOES NOT FIT %s %d \n", file,line);
	}
#define ensure_memory_block(BLOCK,SIZE)__ensure_mem_block_size__(BLOCK,SIZE,__FILE__,__LINE__)
	static inline void init_memory_block(MemoryBlock* block,uint size)
	{
		block->size = size;
		block->start = malloc(size);
		ASSERT_MESSAGE(block->start,"MEMORY CANT BE ALLOCATED");
	}
	static inline void dispose_memory_block(MemoryBlock* block)
	{
		free(block->start);
		block->size = 0;
		block->currentIndex = 0;
	}

	static inline void* get_next_memory_block(const MemoryBlock& block)
	{
		return VOIDPTRINC(block.start,block.currentIndex);
	}

	void _increase_memory_block_(MemoryBlock* block,int add,const char* file,int line)
	{
		block->currentIndex += add;	
		ASSERT_MESSAGE(block->currentIndex < block->size,
				"MEMORY ENDED IN FILE %s \n LINE %d \n,with over %d bytes \n",
				file,line, block->currentIndex- block->size );
	}
#define increase_memory_block(BLOCK,ADD) _increase_memory_block_(BLOCK,ADD,__FILE__,__LINE__)


	void _increase_memory_block_aligned_(MemoryBlock* block,
			int add,const char* file,int line)
	{
		int leftOver = (add+block->currentIndex) % 4;
		int realAdd = add + leftOver;
		block->currentIndex += realAdd;	
		//ASSERT_MESSAGE(block->currentIndex < block->size,"MEMORY ENDED IN FILE %s LINE %d",file,line);
		ASSERT_MESSAGE(block->currentIndex < block->size,
				"MEMORY ENDED IN FILE %s \n LINE %d \n,with over %d bytes \n",
				file,line, block->currentIndex- block->size );

	}
#define increase_memory_block_aligned(BLOCK,ADD) _increase_memory_block_aligned_(BLOCK,ADD,__FILE__,__LINE__)
#


#define IS_TABLE_START(PAIR) ((pair - Table) > TABLE_LENGHT)
#define TABLE_KEY_TO_STRING(TABLE,PAIR) (char*)((int8*)TABLE->table + PAIR->key)
	template<typename T>
		struct StringTable
		{
			//DynamicArray<Pair<T>*>  activepairs;
			Pair<T>* 	table = NULL;//[TABLE_MAX_SIZE]; //= {0};
			uint 		stringPoolSize = 0;
			uint 		overFlowSize = 0;
			uint 		maxStringPoolSize = 0;	
		};
	template<typename T>
		uint 
		init_table(StringTable<T>* table,void* mem,uint desiredSize)
		{
			table->table = (Pair<T>*)mem;
			table->maxStringPoolSize = desiredSize * DEFAULT_STRING_SIZE;
			int size = TABLE_MAX_SIZE * sizeof(Pair<T>) + DEFAULT_STRING_SIZE * desiredSize; 
			memset(table->table,0,size);
			return size; 
		}
	template<typename T>
		void 
		init_table_with_block(StringTable<T>* table,MemoryBlock* mem,uint desiredSize)
		{
			table->table = (Pair<T>*)get_next_memory_block(*mem);//mem->get_next();		
			table->maxStringPoolSize = desiredSize * DEFAULT_STRING_SIZE;
			int size =TABLE_MAX_SIZE * sizeof(Pair<T>) + DEFAULT_STRING_SIZE * desiredSize; 
			increase_memory_block_aligned(mem,size);//mem->increase(size);	
			memset(table->table,0,size);
		}

#define increase_staticAllocator_size(ALLOCATOR,SIZE) _increase_staticAllocator_size_(ALLOCATOR,SIZE,__FILE__,__LINE__)
	struct StaticAllocator;
	inline void* get_next_memory_staticAllocator(StaticAllocator allocator);
	inline void _increase_staticAllocator_size_(StaticAllocator* allocator,uint size,const char* file,int line);
	struct StaticAllocator
	{
		void* 	start = NULL;
		int 	currentSize = 0;
		int 	allocatedSize = 0;
	};
	template<typename T>
		void 
		init_table_with_staticAllocator(StringTable<T>* table,StaticAllocator* mem,uint desiredSize)
		{
			table->table = (Pair<T>*)get_next_memory_staticAllocator(*mem);//get_next_memory_block(&mem);//mem->get_next();		
			table->maxStringPoolSize = desiredSize * DEFAULT_STRING_SIZE;
			int size =TABLE_MAX_SIZE * sizeof(Pair<T>) + DEFAULT_STRING_SIZE * desiredSize; 
			increase_staticAllocator_size(mem,size);
			memset(table->table,0,size);
		}
	static inline int hash(const char* s) 
	{
		long hash = 0;
		const int len_s = (int)strlen(s);
		for (int i = 0; i < len_s; i++) 
		{
			hash += (long)pow(HASH_CODE, len_s - (i+1)) * s[i];
			hash = hash % TABLE_LENGHT;
		}
		return (int)hash;
	}

	template<typename T>
		int 
		insert_to_table(StringTable<T>* table,const char* name,const T& val)
		{
			int loc = hash(name);
			Pair<T>* bucket = NULL;	

			if(table->table[loc].key == 0)
			{
				bucket = &table->table[loc];
			}	
			else
			{
				Pair<T>* parent = &table->table[loc];
				while(parent->link != 0)
				{
					parent = &table->table[parent->link];	
				}
				parent->link = TABLE_LENGHT + table->overFlowSize;//(Pair<T>*)calloc(1,sizeof(Pair<T>));
				table->overFlowSize++;
				ASSERT_MESSAGE(table->overFlowSize < TABLE_OVERFLOW_SIZE,"TABLE OVERFLOW");
				bucket = &table->table[parent->link];
			}		
			int nl =  (int)strlen(name) + 1;
			//int tableByteIndex = (int8)(table->table + TABLE_MAX_SIZE) + table->stringPoolSize; 
			int tableByteIndex = TABLE_MAX_SIZE*sizeof(Pair<T>) + table->stringPoolSize; 
			char* key = (char*)table->table + tableByteIndex;//(char*)malloc(sizeof(char) * (nl+ sizeof(char)));
			memcpy((void*)key, (void*)name , sizeof(char) * (nl + sizeof(char)));
			table->stringPoolSize += nl;
			ASSERT_MESSAGE(table->stringPoolSize < table->maxStringPoolSize,"TABLE STRING MEMORY OVERFLOW");
			bucket->key = tableByteIndex;
			bucket->val = val;
			return loc;
			//char* k = TABLE_KEY_TO_STRING(table,bucket);
		}
	template<typename T>
		T* 
		access_table(const StringTable<T>& table,const char* key)
		{
			int loc = hash(key);
			Pair<T>* bucket = NULL;	

			Pair<T>* parent = &table.table[loc];	
			//ASSERT_MESSAGE(parent->key != NULL,"KEY %s NOT FOUND IN TABLE",key);
			if (parent->key == 0) return NULL;
			char* compKey =TABLE_KEY_TO_STRING((&table),parent);
			if(parent->link == 0 || !strcmp(compKey,key)) return &parent->val;

			bool found = false;
			while( parent != NULL)
			{
				compKey = TABLE_KEY_TO_STRING((&table),parent);
				if(!strcmp(compKey,key))
				{
					found = true;
					break;
				}
				parent = &table.table[parent->link];
			}
			if(!found) return NULL;

			bucket = parent;
			return &bucket->val;
		}

	//TODO test stringtable
	//Static allocator can be used inside different systems, 
	//for example graphics can use it for storing material stringtables


	void init_static_allocator(StaticAllocator* allocator,unsigned int size)
	{
		allocator->currentSize = 0;
		allocator->allocatedSize = size;
		allocator->start = calloc(size,0);
	}

	void init_staticAllocator_with_block(StaticAllocator* allocator,MemoryBlock* block,int size)
	{
		allocator->start = get_next_memory_block(*block);//block->get_next();	
		allocator->allocatedSize = size;
		increase_memory_block(block,size);
	}
	inline void* get_memory_from_staticAllocator(StaticAllocator* allocator,
			unsigned int size)
	{
		void* ret = (void*)((_byte*)allocator->start + allocator->currentSize);	
		allocator->currentSize += size; 
		ASSERT_MESSAGE(allocator->currentSize < allocator->allocatedSize,"STATIC MEMORY ENDED");
		return ret;
	}
	inline void* get_memory_from_staticAllocator_aligned(StaticAllocator* allocator,
			unsigned int size)
	{
		void* ret = (void*)((_byte*)allocator->start + allocator->currentSize);	
		int addOn = (size + allocator->currentSize) % 4;
		allocator->currentSize += size + addOn; 
		ASSERT_MESSAGE(allocator->currentSize < allocator->allocatedSize,"STATIC MEMORY ENDED");
		return ret;
	}

	inline void* get_next_memory_staticAllocator(StaticAllocator allocator)
	{
		return VOIDPTRINC(allocator.start,allocator.currentSize);
	}
	inline void _increase_staticAllocator_size_(StaticAllocator* allocator,uint size,const char* file,int line)
	{
		allocator->currentSize += size;
		ASSERT_MESSAGE(allocator->currentSize < allocator->allocatedSize,
				"STATIC ALLOCATOR OVERFLOW FILE %s, LINE %d",file,line);
	}
#if 0
	struct GPAPointer
	{
		uint index = 0;
		uint size;
	};
	struct GPALinkedList
	{
		uint next = 0;	
		uint size = 0;
		uint myIndex = 0;
	};
	struct GeneralPurposeAllocator
	{
		void*							mem = NULL;
		uint							currentIndex = 0;
		uint							allocatedSize = 0;
		uint							numPointers = 0;
		uint							pointerIndex = 0;
		//freelist of pointers
		DynamicArray<GPALinkedList*>	freeList;
	};
#define DEFAULT_POINTERPOOL_SIZE 100
	static void* access_gpa(const GeneralPurposeAllocator& gpa,const GPAPointer ptr);
	void free_memory_from_gpa(GeneralPurposeAllocator* gpa,GPAPointer ptr);	

	void init_gpa(GeneralPurposeAllocator* gpa,uint size)
	{
		memset(gpa,0,size);
		gpa->mem = malloc(size);
		ASSERT_MESSAGE(gpa->mem,"MEMORY COULD NOT BE ALLOCATED");
		gpa->allocatedSize = size;
		gpa->numPointers = DEFAULT_POINTERPOOL_SIZE;
		init_dynamic_array(&gpa->freeList,DEFAULT_POINTERPOOL_SIZE);
	}
	void dispose_gpa(GeneralPurposeAllocator* gpa)
	{
		free(gpa->mem); 
		gpa->mem = NULL;
		dispose_dynamic_array(&gpa->freeList);
	}
	//[ - - DATA - -, FREELIST POINTERS, linkedlist]
	static inline void* access_gpa(const GeneralPurposeAllocator& gpa,const GPAPointer ptr)
	{	
		uint realIndex = (((GPALinkedList*)((ubyte*)gpa.mem + gpa.allocatedSize)) - ptr.index)->myIndex; //((GPAPointer*)((ubyte*)gpa.mem + gpa.allocatedSize))->index;
		return (void*)((ubyte*)gpa.mem + realIndex);
	}
#define GPA_NULL_PTR 9999
	static void reorder_memory(GeneralPurposeAllocator* gpa)
	{
		GPALinkedList* list = (GPALinkedList*)access_gpa(*gpa,gpa->linkedListPointers);
		GPAPointer* pointers = (GPAPointer*)access_gpa(*gpa,gpa->blockPool);
		GPALinkedList* listIter = list; 
		GPAPointer* pointerIter = pointers; 
		void* moveHere  = NULL;
		int lastIndex = 0;
		int sizeOfMove = 0;
		int startPointerIndex = 0;
		int numIndexesToBeMoved = 0;
		void* start = NULL;
		int sizeOfCopy = 0;
		for(int i = 0 ; listIter ; listIter++,pointerIter++ ,i++)
		{
			//kelaa niin kauan ku löytyy oikea väli
			while(listIter->next - lastIndex == 1)
			{
				lastIndex = listIter->next;
				++listIter; ++i;++pointerIter;
			}
			//ollaan vikassa eli brake
			if(listIter->next == 0) break;
			numIndexesToBeMoved = listIter->next - lastIndex;
			listIter += numIndexesToBeMoved;
			i += numIndexesToBeMoved;


			//moveHere = VOIDPTRINC(gpa->mem,pointers[lastIndex].index);
			startPointerIndex = i;
			start = VOIDPTRINC(gpa->mem,listIter->next);
			sizeOfMove = (int)((int8*) start - (int8*)moveHere);
			lastIndex = listIter->next;
			listIter++;
			sizeOfCopy = pointerIter->size;
			//kelaa niin kauan ku blokkia riittää
			while(listIter->next - lastIndex == 1)
			{
				size += pointerIter->size;
				lastIndex = listIter->next;
				++listIter; ++i;++pointerIter;
			}
			// siirrä muisti lohko ja ota pointerit uudestaan koska ne on voinut muuttua
			memcpy(moveHere,start,size);
			int numMovedIndexes = i -startPointerIndex;
			for(int i2 = 0; i2 < numMovedIndexes; i2++)
			{

			}
		}
		//TODO puhdista vanhat pojat sieltä pois + mieti elämää
		//
		//aika masentava
	}

	GPAPointer get_memory_from_gpa(GeneralPurposeAllocator* gpa,uint size)
	{
		GPALinkedList* pointer = NULL; 
		if(gpa->freeList.numobj == 0)
		{
			if(gpa->pointerIndex + 1 > gpa->numPointers)
			{
				gpa->numPointers *= 2;
				uint sizeNeeded = gpa->numPointers * (sizeof(GPALinkedList) + sizeof(GPALinkedList*));
				if(sizeNeeded > (gpa->currentIndex - gpa->allocatedSize))
				{
					// reorder memory
					reorder_memory(gpa);
				}
				ASSERT_MESSAGE(sizeNeeded < (gpa->currentIndex - gpa->allocatedSize),"GPA MEMORY ENDED");
			}
			gpa->pointerIndex++;
			pointer = ((GPALinkedList*)VOIDPTRINC(gpa->mem,gpa->allocatedSize)) - gpa->pointerIndex;
		}
		else
		{
			gpa->freeList.numobj--;
			pointer  = gpa->freeList.buffer[gpa->freeList.numobj];
		}
		GPAPointer userPointer;
		int pointerIndex = (((GPALinkedList*)((int8*)gpa->mem + gpa->allocatedSize)) - pointer); 
		userPointer.index = pointerIndex;
		userPointer.size = size;
		pointer->size = size;
		pointer->next = GPA_NULL_PTR;

#if 0
		GPAPointer ptrToPool; 	
		GPAPointer ptrToUser; 	
		ptrToPool.size = size;
		ptrToUser.size = size;
		if(gpa->indexForPoolAndLinked + 1 < gpa->linkedListPointers.size)
		{
			//realloc pointers and linked list
			int newSize  = gpa->linkedListPointers.size * 2;
			int sizeNeededLinked = sizeof(GPALinkedList) * newSize;
			int sizeNeededPointers = sizeof(GPAPointer) * newSize;

			if(gpa->currentIndex + sizeNeededLinked + sizeNeededPointers < gpa->allocatedSize)
			{
				//fjasöfjasö	
				//reorder and check
			}

		}
		if(size + gpa->currentIndex > gpa->allocatedSize)
		{
			//reorder and check for memory
		}
		GPALinkedList* linkedpointers = (GPALinkedList*)access_gpa(*gpa,gpa->linkedListPointers);
		GPALinkedList* realLink = &linkedpointers[gpa->indexForPoolAndLinked];
		ptrToUser.index = gpa->indexForPoolAndLinked;
		GPAPointer* pointers = (GPAPointer*)access_gpa(*gpa,gpa->linkedListPointers);

#endif
		return userPointer;
	}
#endif
}
#endif // CONTAINERS_H

