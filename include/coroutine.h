#pragma once
#include <functional>
#include "context.h"
#include "utils.h"
#include "logger.h"
/**
 * minico会根据计算机的核心数开对应的线程数来运行协程，其中每一个线程对应一个Processor实例
 * 协程Coroutine实例运行在Processor的主循环上，Processor使用Epoll和Timer进行任务调度
 * Scheduler是一个全局单例，当某个线程中调用co_go()运行一个新协程后实际就会调用
 * 该实例的方法，选择一个协程最少的Preocessor来接管这个协程
 */
namespace minico
{
	enum coStatus
	{
		CO_READY = 0,
		CO_RUNNING,
		CO_WAITING,
		CO_DEAD
	};
	class Processor;

	class Coroutine
	{
	public:
		Coroutine(Processor*, size_t stackSize, std::function<void()>&&);
		Coroutine(Processor*, size_t stackSize, std::function<void()>&);
		~Coroutine();

		DISALLOW_COPY_MOVE_AND_ASSIGN(Coroutine);

		/** 恢复当前协程*/
		void resume();

		/** 切出当前协程*/
		void yield();

		/** 获得当前处理器指针*/
		Processor* getMyProcessor(){return pMyProcessor_;}

		/** 开始运行协程任务*/
		inline void startFunc() { coFunc_(); }

		/** 获得当前协程上下文*/
		inline Context* getCtx() { return &ctx_; }

	private:

		/** 协程搭载的任务*/
		std::function<void()> coFunc_;		

		/** 协程对应的处理器*/
		Processor* pMyProcessor_;		

		/** 协程状态*/
		int status_;				

		/** 协程上下文*/
		Context ctx_;				

	};

}
