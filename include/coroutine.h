//@author Liu Yukang
#pragma once
#include <functional>
#include "context.h"
#include "utils.h"
/**
 * netco会根据计算机的核心数开对应的线程数来运行协程，其中每一个线程对应一个Processor实例
 * 协程Coroutine实例运行在Processor的主循环上，Processor使用Epoll和Timer进行任务调度
 * 而Scheduler则并不存在这么一个循环，它是一个全局单例，当某个线程中调用co_go()运行一个新协程后
 * 实际就会调用该实例的方法，选择一个协程最少的Preocessor来接管这个协程
 */
namespace minico
{
	//协程的四种状态 分别为0 1 2 3
	enum coStatus
	{
		CO_READY = 0,
		CO_RUNNING,
		CO_WAITING,
		CO_DEAD
	};

	class Processor;
	///@协程类封装
	class Coroutine
	{
	public:
		Coroutine(Processor*, size_t stackSize, std::function<void()>&&);
		Coroutine(Processor*, size_t stackSize, std::function<void()>&);
		~Coroutine();

		DISALLOW_COPY_MOVE_AND_ASSIGN(Coroutine);

		//恢复运行当前协程
		void resume();

		//暂停运行当前协程
		void yield();

		Processor* getMyProcessor(){return pMyProcessor_;}

		//运行该协程的函数
		inline void startFunc() { coFunc_(); }

		//获取该协程的上下文
		inline Context* getCtx() { return &ctx_; }

	private:

		std::function<void()> coFunc_;		//封装的工作函数

		Processor* pMyProcessor_;		//对应的处理器核心

		int status_;				//协程的状态

		Context ctx_;				//协程的上下文结构

	};

}
