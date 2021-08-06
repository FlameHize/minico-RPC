#pragma once
#include <queue>
#include <set>
#include <mutex>
#include <thread>
#include "objpool.h"
#include "spinlock.h"
#include "context.h"
#include "coroutine.h"
#include "epoller.h"
#include "timer.h"
/**
 * @brief 处理器（对应一个CPU的核心，在netco中对应一个线程） 其实质就是一个线程
 * @detail 负责存放协程Coroutine实例并管理其生命期
 * @param[in] newCoroutines_ 新协程双缓冲队列，使用一个队列来存放新来的协程，另一个队列给Processor主循环用于执行新来的协程，消费完成以后就交换队列（每加入一次新协程就唤醒一次主循环，以立即执行新来的协程）
 * @param[in] actCoroutines_ 被epoll激活的协程队列.当epoll_wait被激活时，Processor主循环会尝试从Epoll中获取活跃的协程，存放在该actCoroutines队列中，然后依次恢复执行
 * @param[in] timerExpiredCo_ 超时的协程队列.当epoll_wait被激活时,Processor主循环会首先尝试从Timer中获取活跃的协程，存放在timerExpiredCo队列中，然后依次恢复执行
 * @param[in] removedCo_ 被移除的协程队列.执行完的协程会首先放到该队列中,在一次循环的最后被统一清理
 * @detail 主循环执行顺序：timer->new->act->remove
 */
extern __thread int threadIdx;		//线程序号

namespace minico
{
	//线程的状态
	enum processerStatus
	{
		PRO_RUNNING = 0,
		PRO_STOPPING,
		PRO_STOPPED
	};
	//是否正在加入新协程
	enum newCoAddingStatus
	{
		NEWCO_ADDING = 0,
		NEWCO_ADDED
	};

	class Processor
	{
	public:
		Processor(int);
		~Processor();

		DISALLOW_COPY_MOVE_AND_ASSIGN(Processor);

		//运行一个新协程，该协程的函数是func
		void goNewCo(std::function<void()>&& func, size_t stackSize);
		void goNewCo(std::function<void()>& func, size_t stackSize);

		void yield();

		//当前协程等待time毫秒
		void wait(Time time);

		//清除当前正在运行的协程
		void killCurCo();
		//线程的事件循环
		bool loop();
		//停止线程循环
		void stop();

		void join();

		//等待fd上的ev事件返回
		void waitEvent(int fd, int ev);

		//获取当前正在运行的协程
		inline Coroutine* getCurRunningCo() { return pCurCoroutine_; };

		inline Context* getMainCtx() { return &mainCtx_; }
		//获得当前线程存在的协程总数量
		inline size_t getCoCnt() { return coSet_.size(); }
		//运行一个指定的协程
		void goCo(Coroutine* co);
		//运行一组指定的协程
		void goCoBatch(std::vector<Coroutine*>& cos);

	private:

		//恢复运行指定协程
		void resume(Coroutine*);
		//唤醒epoll
		inline void wakeUpEpoller();

		//该处理器的线程号
		int tid_;
		//线程的状态
		int status_;
		//std给定的线程对象
		std::thread* pLoop_;

		//新任务队列，使用双缓存队列(一个队列来存放新来的协程,另一个队列给线程事件循环用于执行新来的协程,消费完后就交换队列)
		std::queue<Coroutine*> newCoroutines_[2];

		//新任务双缓存队列中正在运行的队列号，另一条用于添加任务
		volatile int runningNewQue_;

		Spinlock newQueLock_;

		Spinlock coPoolLock_;

		//std::mutex newCoQueMtx_;

		//EventEpoller发现的活跃事件所放的列表
		std::vector<Coroutine*> actCoroutines_;

		std::set<Coroutine*> coSet_;

		//定时器任务列表
		std::vector<Coroutine*> timerExpiredCo_;

		//被移除的协程列表，要移除某一个事件会先放在该列表中，一次循环结束才会真正delete
		std::vector<Coroutine*> removedCo_;

		Epoller epoller_;                     //EPOLL控制器

		Timer timer_;                         //定时器

		ObjPool<Coroutine> coPool_;           //对象池

		Coroutine* pCurCoroutine_;            //当前协程

		Context mainCtx_;                     //用于进行调度保存的mainCTX上下文结构体
	};

}

