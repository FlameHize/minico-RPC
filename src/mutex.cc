#include "../include/mutex.h"
#include "../include/scheduler.h"
#include "../include/spinlock_guard.h"

using namespace minico;

void RWMutex::rlock()
{
    {
        SpinlockGuard l(lock_);				//上锁
        if(state_ == MU_FREE || state_ == MU_READING)
	{
            readingNum_.fetch_add(1);
	    //作用：原子替换(obj + arg -> obj), 并返回obj之前的值
            state_ = MU_READING;
            return;
        }
        //相当于互斥了 就将当前没有上读锁的协程放入协程队列
        waitingCo_.push(Scheduler::getScheduler()->getProcessor(threadIdx)->getCurRunningCo());		
    }
    Scheduler::getScheduler()->getProcessor(threadIdx)->yield();					//当前协程让出CPU
    rlock();												//切换回来会再上读锁
}

void RWMutex::runlock()
{
    SpinlockGuard l(lock_);
    auto cur = readingNum_.fetch_add(-1);
    if(cur == 1)
    {
        freeLock();											//如果读锁的引用计数为0 那么就唤醒互斥队列（可能存在写锁）
    }
}

void RWMutex::wlock()
{    
    {
        SpinlockGuard l(lock_);
        if(state_ == MU_FREE)
	{
            state_ = MU_WRITING;
            return;
        }
        waitingCo_.push(Scheduler::getScheduler()->getProcessor(threadIdx)->getCurRunningCo());
    }
    Scheduler::getScheduler()->getProcessor(threadIdx)->yield();
    wlock();
}
//直接释放锁（也就是互斥队列中可能有读锁）
void RWMutex::wunlock()
{
    SpinlockGuard l(lock_);
    freeLock();
}

void RWMutex::freeLock()
{
    state_ = MU_FREE;
    while(!waitingCo_.empty()){
        auto wakeCo = waitingCo_.front();
        waitingCo_.pop();
        wakeCo->getMyProcessor()->goCo(wakeCo);
    }
}