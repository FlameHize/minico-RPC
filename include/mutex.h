#pragma once
#include "coroutine.h"
#include "spinlock.h"

#include <atomic>
#include <queue>

namespace minico
{

    enum muStatus
	{
		MU_FREE = 0,
		MU_READING,
		MU_WRITING
	};

    /** 用于协程同步的读写锁,读锁互相不互斥而与写锁互斥，写锁与其他的均互斥*/
    class RWMutex
    {
    public:
        RWMutex()
            : state_(MU_FREE), readingNum_(0)
        {};
        ~RWMutex(){};

        DISALLOW_COPY_MOVE_AND_ASSIGN(RWMutex);


        void rlock();

        void runlock();


        void wlock();

        void wunlock();

    private:
        void freeLock();

        int state_;					

        std::atomic_int readingNum_;			

        Spinlock lock_;				
  
        std::queue<Coroutine*> waitingCo_;	

    };

}