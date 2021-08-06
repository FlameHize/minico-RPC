#include "../include/timer.h"
#include "../include/coroutine.h"
#include "../include/epoller.h"

#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <string.h>
#include <unistd.h>

using namespace minico;

Timer::Timer()
	: timeFd_(-1)
{}

Timer::~Timer() 
{
	if (isTimeFdUseful())
	{
		::close(timeFd_);
	}
}

bool Timer::init(Epoller* pEpoller)
{
	timeFd_ = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (isTimeFdUseful())
	{
		return pEpoller->addEv(nullptr, timeFd_, EPOLLIN | EPOLLPRI | EPOLLRDHUP);	//在epoll中加入对该timerfd的监听
	}
	return false;
}

void Timer::getExpiredCoroutines(std::vector<Coroutine*>& expiredCoroutines)
{
	Time nowTime = Time::now();
	while (!timerCoHeap_.empty() && timerCoHeap_.top().first <= nowTime)
	{
		expiredCoroutines.push_back(timerCoHeap_.top().second);
		timerCoHeap_.pop();
	}
	//如果超时的协程队列非空（也就是有超时的协程）
	if (!expiredCoroutines.empty())
	{
		ssize_t cnt = TIMER_DUMMYBUF_SIZE;
		while (cnt >= TIMER_DUMMYBUF_SIZE)
		{
			cnt = ::read(timeFd_, dummyBuf_, TIMER_DUMMYBUF_SIZE);	//读取timerfd上数据的缓冲区
		}
	}
	//重新设置定时的时间
	if (!timerCoHeap_.empty())
	{
		Time time = timerCoHeap_.top().first;
		resetTimeOfTimefd(time);
	}
}
//在time时刻需要恢复协程co
void Timer::runAt(Time time, Coroutine* pCo)
{
	timerCoHeap_.push(std::move(std::pair<Time, Coroutine*>(time, pCo)));
	if (timerCoHeap_.top().first == time)
	{//新加入的任务是最紧急的任务则需要更改timefd所设置的时间
		resetTimeOfTimefd(time);
	}
}

//给timefd重新设置时间，time是绝对时间
bool Timer::resetTimeOfTimefd(Time time)
{
	struct itimerspec newValue;
	struct itimerspec oldValue;
	memset(&newValue, 0, sizeof newValue);
	memset(&oldValue, 0, sizeof oldValue);
	newValue.it_value = time.timeIntervalFromNow();
	int ret = ::timerfd_settime(timeFd_, 0, &newValue, &oldValue);
	return ret < 0 ? false : true;
}
//经过time毫秒恢复协程co
void Timer::runAfter(Time time, Coroutine* pCo)
{
	Time runTime(Time::now().getTimeVal() + time.getTimeVal());
	runAt(runTime, pCo);
}
///@唤醒timerfd
void Timer::wakeUp()
{
	resetTimeOfTimefd(Time::now());
}