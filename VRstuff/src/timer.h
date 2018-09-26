#ifndef PAKKI_TIMER
#define PAKKI_TIMER

#include <ctime>
#include <ratio>
#include <chrono>
#include <Utils.h>
//#include <Containers.h>

namespace PROFILER
{

#define TIMER_IDS(FILE)\
	FILE(Render)\
	FILE(Start)\
	FILE(Iteration)\


	const char* TIMER_NAMES[] = {
		TIMER_IDS(GENERATE_STRING)
	};

	enum class TimerID : int
	{
		TIMER_IDS(GENERATE_ENUM)
			Max
	};

	struct Timer
	{
		std::chrono::high_resolution_clock::time_point 	t1;	
		std::chrono::duration<double>					average;
		std::chrono::duration<double>					low;
		std::chrono::duration<double>					high;
		uint 											numCalls = 0;
	};
	struct TimerCache
	{
		Timer					_Timers[(int)TimerID::Max] ;
	};

	void init_timers(TimerCache* timers)
	{
		memset(timers->_Timers , 0 ,sizeof(TimerCache));
	}
	inline void start_timer(TimerID id,TimerCache* timers)
	{
		Timer* t = &timers->_Timers[(int)id];
		t->t1 = std::chrono::high_resolution_clock::now();
	}
	inline void end_timer(TimerID id,TimerCache* timers)
	{
		Timer* t = &timers->_Timers[(int)id];
		std::chrono::duration<double> ntime = std::chrono::duration_cast<std::chrono::duration<double>>
			(std::chrono::high_resolution_clock::now() - t->t1);
		if(ntime < t->low || t->numCalls == 0)
		{
			t->low = ntime;
		}
		else if (ntime > t->high )//|| t->high.count() == 0)
		{
			t->high = ntime;	
		}
		t->average += ntime; 
		t->numCalls++; 
	}
	void show_timers(TimerCache* timers)
	{
		for(int i = 0; i < (int)TimerID::Max; i++){
			double average = timers->_Timers[i].average.count() 
				/ (double)(timers->_Timers[i].numCalls == 0 ? 1 : timers->_Timers[i].numCalls);	

			printf("%s min [%f]  max[%f] average[%f] , numCalls [%d]\n", 
					TIMER_NAMES[i],timers->_Timers[i].low.count(),timers->_Timers[i].high.count(),average,
					timers->_Timers[i].numCalls);
		}
	}

}
#endif // TIMER_IMPLEMENTATION
