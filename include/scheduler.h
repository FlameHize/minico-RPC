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

		static Scheduler* getScheduler();

		void createNewCo(std::function<void()>&& func, size_t stackSize);
		void createNewCo(std::function<void()>& func, size_t stackSize);

		Processor* getProcessor(int);

		int getProCnt();

		void join();

	private:
		bool startScheduler(int threadCnt);

		static Scheduler* pScher_;

		static std::mutex scherMtx_;

		std::vector<Processor*> processors_;

		ProcessorSelector proSelector_;
	};

}
