#include "../include/minico_api.h"
#include "../../include/logger.h"

void minico::co_go(std::function<void()>&& func, size_t stackSize, int tid)
{
	if (tid < 0)
	{
		LOG_INFO("CREATE A CO");
		minico::Scheduler::getScheduler()->createNewCo(std::move(func), stackSize);
	}
	else
	{
		tid %= minico::Scheduler::getScheduler()->getProCnt();
		minico::Scheduler::getScheduler()->getProcessor(tid)->goNewCo(std::move(func), stackSize);
	}
}

void minico::co_go(std::function<void()>& func, size_t stackSize, int tid)
{
	if (tid < 0)
	{
		LOG_INFO("CREATE A CO");
		minico::Scheduler::getScheduler()->createNewCo(func, stackSize);
	}
	else
	{
		tid %= minico::Scheduler::getScheduler()->getProCnt();
		minico::Scheduler::getScheduler()->getProcessor(tid)->goNewCo(func, stackSize);
	}
}

void minico::co_sleep(Time time)
{
	minico::Scheduler::getScheduler()->getProcessor(threadIdx)->wait(time);
}

void minico::sche_join()
{
	minico::Scheduler::getScheduler()->join();
}