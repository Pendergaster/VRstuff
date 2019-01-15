#ifndef PAKKI_UTILS
#define PAKKI_UTILS
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <malloc.h>

#define CONCAT_INTERNAL(x,y) x##y
#define CONCAT(x,y) CONCAT_INTERNAL(x,y)

#define LOG_STR "LOG: "

#define LOG(...) do{printf(LOG_STR __VA_ARGS__ );printf(" in file : %s:%d \n", __FILE__, __LINE__); \
}while(0)
#define ASSERT_MESSAGE(condition,...) do{ if(!(condition)){  LOG(__VA_ARGS__); assert(condition);}}while(0) 
#define ABORT_MESSAGE(...) do{  LOG(__VA_ARGS__);fflush(stdout); assert(false); }while(0) 
#define VOIDPTRINC(PTR,SIZE) ((void*)((int8*)PTR + SIZE))

#define GENERATE_STRING(STRING) #STRING,

#define GENERATE_ENUM(ENUM) ENUM,

#ifdef PAKKI_DEBUG
#define IF_DEBUG(...) __VA_ARGS__ 
#else
#define IF_DEBUG(...)
#endif // PAKKI_DEBUG

#define BIT_CHECK(a,b) ((a & b) > 0) // onks oikein?
#define BIT_SET(a,b) ( a |= b)
#define BIT_UNSET(a,b) (a &= ~b)
#define BETWEEN(a,x,b) (a < x && b > x)
#define LIMIT(x,a,b) do {x = x < a ? a : x;x = x > b ? b : x;}while(0)
#define BIT_FLIP(a,b) (a ^= b)
#define IF_ELE(cond,a,b) do{ if(cond){a;} else {b;} }while(0)
#define IF_CHECK(cond,a) do{ if(cond){a;} }while(0)

typedef uint8_t		ubyte;
typedef int8_t		int8;
typedef uint32_t	uint;

int MEMTRACK = 0;

inline void* DEBUG_MALLOC(uint size)
{
	MEMTRACK++;
	return malloc(size);
}

inline void* DEBUG_CALLOC(int COUNT, int SIZE)
{
	MEMTRACK++;
	return calloc(COUNT, SIZE);
}

#define surffi_aki(PTR) do{ free(PTR); MEMTRACK--;}while(0)
#define neo_aki(PTR) do{ free(PTR); MEMTRACK--;}while(0)

//#define MEM_DEBUG
#ifdef  MEM_DEBUG
#define free(PTR) do{ free((PTR)); MEMTRACK--;}while(0)
#define malloc(SIZE) DEBUG_MALLOC((SIZE))
#define calloc(COUNT,SIZE) DEBUG_CALLOC((COUNT),(SIZE))
#endif //  MEM_DEBUG

#define CHECK_MEM() ASSERT_MESSAGE(MEMTRACK == 0, "MEMORY NOT BALANCED, %d \n",MEMTRACK)


//https://pastebin.com/3YvWQa5c
template<typename T>
struct CleanUP {
    T lambda;
    CleanUP(T lambda):lambda(lambda){}
    ~CleanUP(){lambda();}
    CleanUP(const CleanUP&);
  private:
    CleanUP& operator =(const CleanUP&);
};

class CleanUpHelp {
  public:
    template<typename T>
        CleanUP<T> operator+(T t){ return t;}
};

#define defer const auto& CONCAT(defer__, __LINE__) = CleanUpHelp() + [&]()

#endif // PAKKI_UTILS
