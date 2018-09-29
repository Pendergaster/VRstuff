#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#define NUMTHREADS 5
#define THREAD_FAILURE(...)

typedef unsigned int uint;

typedef struct
{
	void* (*job)(void*);
} threadJob;

#define NUM_THREADS 4

typedef struct
{
	pthread_t				threads[NUM_THREADS];
	pthread_mutex_t			queMutex; 
	uint					numJobs;
	uint					jobIndex;
	threadJob*				jobs;
} threadPool;

#define NUM_JOBS 10

void* job_func(void *);


static inline void init_threadPool(threadPool* pool,uint numThreads)
{
	pool->jobs = (threadJob*)malloc(sizeof(threadJob) * NUM_JOBS);
	pool->jobIndex = 0;
	pool->numJobs = NUM_JOBS;
	if(pthread_mutex_init(&pool->queMutex,NULL)) {
		THREAD_FAILURE();
	}

	}

void* job_func(void* ptr)
{
	threadPool* home = (threadPool*)ptr;
	pthread_cond_t cond;

	if(pthread_cond_init(&cond, NULL)){
		THREAD_FAILURE();
	}
	pthread_mutex_lock(&home->queMutex);

	pthread_cond_wait(&cond,&home->queMutex);

	if(pthread_cond_destroy(&cond)){
		THREAD_FAILURE();
	}
	//TODO käynnistä threadit, cond variables ja atomicit
	
	return 0;
}

pthread_mutex_t mutex;

void* foo(void* ptr)
{
	pthread_mutex_lock(&mutex);
	printf("morot \n");
	pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);
}

int main()
{

	if(pthread_mutex_init(&mutex,NULL)){
		printf("failed to init mutex \n");
	}

	pthread_t threads[NUMTHREADS];
	
	for(pthread_t* i = threads; i < &threads[NUMTHREADS]; i++)
	{
		int check = pthread_create( i, NULL, foo, NULL);
		if(check)
		{
			printf("FAILED TO CREATE THREAD \n");
		}
	}

	for(pthread_t* i = threads; i < &threads[NUMTHREADS]; i++)
	{
		pthread_join(*i,NULL);
		printf("%ld joined \n",i - threads);
	}

	pthread_mutex_destroy(&mutex);
	return 0;
}
