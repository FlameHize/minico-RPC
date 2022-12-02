#pragma once
#include <vector>
#include <functional>

#include "processor.h"
#include "processor_selector.h"
/**
 * @brief 让用户指定协程运行在某个Processor上，若用户没有指定，则挑选协程数量最少的Processor接管新的协程
 */
namespace minico
{

	class Scheduler
	{
	protected:
		Scheduler();
		~Scheduler();

	public:

		DISALLOW_COPY_MOVE_AND_ASSIGN(Scheduler);

		/** 单例模式 获得一个协程调度器*/
		static Scheduler* getScheduler();

		/** 创建新协程*/
		void createNewCo(std::function<void()>&& func, size_t stackSize);
		void createNewCo(std::function<void()>& func, size_t stackSize);

		/** 获得指定编号的处理器*/
		Processor* getProcessor(int);

		/** 获得当前管理处理器的数量*/
		int getProCnt();

		/** 停止运行*/
		void join();

	private:

		/** 创建threadCnt个处理器并开启事件循环*/
		bool startScheduler(int threadCnt);

		/** 全局唯一的协程管理器实例*/
		static Scheduler* pScher_;

		/** 互斥锁*/
		static std::mutex scherMtx_;

		/** 调度的处理器队列*/
		std::vector<Processor*> processors_;

		/** 协程分发器*/
		ProcessorSelector proSelector_;
	};

}
