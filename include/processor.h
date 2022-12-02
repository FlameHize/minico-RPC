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
#include "logger.h"
/**
 * @brief 处理器（对应一个CPU的核心，在netco中对应一个线程） 其实质就是一个线程
 * @detail 负责存放协程Coroutine实例并管理其生命期
 * @param[in] newCoroutines_ 新协程双缓冲队列，使用一个队列来存放新来的协程，另一个队列给Processor主循环用于执行新来的协程，消费完成以后就交换队列（每加入一次新协程就唤醒一次主循环，以立即执行新来的协程）
 * @param[in] actCoroutines_ 被epoll激活的协程队列.当epoll_wait被激活时，Processor主循环会尝试从Epoll中获取活跃的协程，存放在该actCoroutines队列中，然后依次恢复执行
 * @param[in] timerExpiredCo_ 超时的协程队列.当epoll_wait被激活时,Processor主循环会首先尝试从Timer中获取活跃的协程，存放在timerExpiredCo队列中，然后依次恢复执行
 * @param[in] removedCo_ 被移除的协程队列.执行完的协程会首先放到该队列中,在一次循环的最后被统一清理
 * @detail 主循环执行顺序：timer->new->act->remove
 */
extern __thread int threadIdx;		

namespace minico
{

	enum processerStatus
	{
		PRO_RUNNING = 0,
		PRO_STOPPING,
		PRO_STOPPED
	};

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

		/** 运行一个新协程，该协程的函数是func*/
		void goNewCo(std::function<void()>&& func, size_t stackSize);
		void goNewCo(std::function<void()>& func, size_t stackSize);

		/** 暂停运行当前协程*/
		void yield();

		/** 当前运行协程等待time毫秒*/
		void wait(Time time);

		/** 销毁当前运行的协程*/
		void killCurCo();
		
		/** 处理器对应线程的主事件循环*/
		bool loop();

		/** 停止线程循环*/
		void stop();
		void join();

		/** 等待fd上的ev事件返回*/
		void waitEvent(int fd, int ev);

		/** 获取正在运行的协程*/
		inline Coroutine* getCurRunningCo() { return pCurCoroutine_; };

		/** 获取当前处理器的主程序上下文*/
		inline Context* getMainCtx() { return &mainCtx_; }

		/** 获取当前处理器存在的协程总数量*/
		inline size_t getCoCnt() { return coSet_.size(); }

		/** 运行一个指定的协程*/
		void goCo(Coroutine* co);

		/** 运行一组指定的协程*/
		void goCoBatch(std::vector<Coroutine*>& cos);

	private:
		/** 恢复运行指定协程*/
		void resume(Coroutine*);

		/** 唤醒epoll */
		inline void wakeUpEpoller();

		/** 处理器编号*/
		int tid_;

		/** 处理器状态*/
		int status_;

		/** 处理器对应线程*/
		std::thread* pLoop_;

		/** 
		 * @brief 双缓冲任务队列
		 * 一个队列存放新加入的协程，另一个用于执行存放的协程
		 * 消费完毕后就交换队列
		 * */
		std::queue<Coroutine*> newCoroutines_[2];

		/** 双缓冲任务队列编号 0 or 1*/
		volatile int runningNewQue_;

		/** 自旋锁，用于切换双缓冲任务队列*/
		Spinlock newQueLock_;

		/** 自旋锁，用于对象池操作*/
		Spinlock coPoolLock_;

		/** 被epoll激活的协程任务队列*/
		std::vector<Coroutine*> actCoroutines_;

		/** 记录当前处理器搭载的所有协程*/
		std::set<Coroutine*> coSet_;

		/** 协程定时任务队列*/
		std::vector<Coroutine*> timerExpiredCo_;

		/** 待销毁协程任务队列*/
		std::vector<Coroutine*> removedCo_;

		/** epoll控制体*/
		Epoller epoller_;                     

		/** 定时器*/
		Timer timer_;                         

		/** 对象池*/
		ObjPool<Coroutine> coPool_;           

		/** 处理器当前运行的协程*/
		Coroutine* pCurCoroutine_;            

		/** 处理器当前程序上下文*/
		Context mainCtx_;                     
	};

}

