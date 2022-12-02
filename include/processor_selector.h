#pragma once
#include <vector>

namespace minico
{
	class Processor;

	enum scheduleStrategy
	{
		MIN_EVENT_FIRST = 0 , 
		ROUND_ROBIN	      
	};

	/** 协程调度策略管理器，决定下一个协程应该放到哪一个线程中*/
	class ProcessorSelector
	{
	public:
		ProcessorSelector(std::vector<Processor*>& processors, int strategy = MIN_EVENT_FIRST) :  curPro_(-1) , strategy_(strategy) , processors_(processors) {}
		~ProcessorSelector() {}

		/** 设置调度策略*/
		inline void setStrategy(int strategy) { strategy_ = strategy; };

		/** 获取被调度的处理器*/
		Processor* next();

	private:
	
		/** 当前处理器编号*/
		int curPro_;

		/** 当前策略*/
		int strategy_;

		/** 调度的处理器队列*/
		std::vector<Processor*>& processors_;

	};

}