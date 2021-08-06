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

    //用于协程同步的读写锁,读锁互相不互斥而与写锁互斥，写锁与其他的均互斥
    //原理：类中维护了一个队列，如果互斥了则将当前协程放入队列中等待另一协程解锁时的唤醒
    class RWMutex
    {
    public:
        RWMutex()
            : state_(MU_FREE), readingNum_(0)
        {};
        ~RWMutex(){};

        DISALLOW_COPY_MOVE_AND_ASSIGN(RWMutex);

        //读锁
        void rlock();
        //解读锁
        void runlock();

        //写锁
        void wlock();
        //解写锁
        void wunlock();

    private:
        void freeLock();

        int state_;					//读写锁的状态

        std::atomic_int readingNum_;			//读锁的使用数量

        Spinlock lock_;					//自旋锁
  
        std::queue<Coroutine*> waitingCo_;		//协程等待队列

    };

}