#pragma once
#include "scheduler.h"
#include "mstime.h"

namespace minico 
{

	/** 
	 * @brief 运行一个协程
	 * @param func 运行的函数任务
	 * @param stackSize 运行的栈大小 默认为2048
	 * @param tid 选取的处理器编号 默认为-1 也就是使用策略调度器选择处理器
	*/
	void co_go(std::function<void()>& func, size_t stackSize = parameter::coroutineStackSize, int tid = -1);
	void co_go(std::function<void()>&& func, size_t stackSize = parameter::coroutineStackSize, int tid = -1);

	void co_sleep(Time t);

	void sche_join();

}


