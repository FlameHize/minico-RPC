#pragma once
#include "mstime.h"
#include "utils.h"

#include <map>
#include <queue>
#include <vector>
#include <mutex>
#include <functional>
/**
 * timefd配合一个小根堆来实现
 * 小根堆存放的是时间和协程对象的pair
 */
#define TIMER_DUMMYBUF_SIZE 1024

namespace minico
{
	class Coroutine;
	class Epoller;

	//��ʱ��
	class Timer
	{
	public:
		using TimerHeap = typename std::priority_queue<std::pair<Time, Coroutine*>, std::vector<std::pair<Time, Coroutine*>>, std::greater<std::pair<Time, Coroutine*>>>;

		Timer();
		~Timer();

		bool init(Epoller*);

		DISALLOW_COPY_MOVE_AND_ASSIGN(Timer);

		void getExpiredCoroutines(std::vector<Coroutine*>& expiredCoroutines);

		void runAt(Time time, Coroutine* pCo);

		void runAfter(Time time, Coroutine* pCo);

		void wakeUp();

	private:

		bool resetTimeOfTimefd(Time time);

		inline bool isTimeFdUseful() { return timeFd_ < 0 ? false : true; };

		int timeFd_;

		char dummyBuf_[TIMER_DUMMYBUF_SIZE];

		TimerHeap timerCoHeap_;
	};

}
