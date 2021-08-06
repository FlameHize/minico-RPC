#include "../include/processor.h"
#include "../include/parameter.h"
#include "../include/spinlock_guard.h"

#include <sys/epoll.h>
#include <unistd.h>

using namespace minico;

__thread int threadIdx = -1;

Processor::Processor(int tid)
	: tid_(tid), status_(PRO_STOPPED), pLoop_(nullptr), runningNewQue_(0), pCurCoroutine_(nullptr), mainCtx_(0)
{
	mainCtx_.makeCurContext();         //将当前的执行上下文保存到mainCtx的ucontext结构体中
}

Processor::~Processor()
{
	if (PRO_RUNNING == status_)
	{
		stop();
	}
	if (PRO_STOPPING == status_)
	{
		join();
	}
	if (nullptr != pLoop_)
	{
		delete pLoop_;
	}
	for (auto co : coSet_)
	{
		delete co;
	}
}

///@唤醒一个指定的协程（在loop中调用）
void Processor::resume(Coroutine* pCo)
{
	if (nullptr == pCo)
	{
		return;
	}

	if (coSet_.find(pCo) == coSet_.end())
	{
		return;
	}
	///@将线程保存的当前协程指针指向该协程
	pCurCoroutine_ = pCo;
	///@实际执行唤醒该协程（将中断现场信息保存到mainCtx中）,执行完该函数之后程序控制流会转移到协程的工作函数startFunc()中
	pCo->resume();
}

///@暂停运行当前协程
void Processor::yield()
{
	///@设置当前执行的协程的状态为CO_WAITING
	pCurCoroutine_->yield();                       
	//将当前执行的协程中断信息保存到其ctx上下文结构体中，并利用mainCtx刷新寄存器
	mainCtx_.swapToMe(pCurCoroutine_->getCtx());
}
///@当前协程等待time(ms)
void Processor::wait(Time time)
{
	pCurCoroutine_->yield();			//暂停该协程（实质只是更改了协程的状态为CO_WAITING）
	timer_.runAfter(time,pCurCoroutine_);		//放入定时器中
	mainCtx_.swapToMe(pCurCoroutine_->getCtx());	///@将当前协程从CPU中切出,使线程保存的上下文结构体类mainCtx_占有CPU并开始执行
}
///@开始运行一个指定的协程pCo
void Processor::goCo(Coroutine* pCo)
{
	{
		SpinlockGuard lock(newQueLock_);	///@争夺的是队列锁,这里是存放新来的协程,这个地方是逻辑的重点,也就是两个队列的核心
		newCoroutines_[!runningNewQue_].push(pCo);	///@重点！！！将协程放入线程中存储,之后唤醒并运行
	}
	///@每加入一个新协程就会唤醒一次Processor主循环,以立即执行新来的协程(因为epoll_wait是阻塞的,否则可能加入了没有执行)
	wakeUpEpoller();
}
//运行一组指定的协程cos
void Processor::goCoBatch(std::vector<Coroutine*>& cos)
{
	{
		SpinlockGuard lock(newQueLock_);
		for(auto pCo : cos)
		{
			newCoroutines_[!runningNewQue_].push(pCo);
		}
	}
	wakeUpEpoller();
}

//线程事件循环
bool Processor::loop()
{
	//初始化Epoller
	if (!epoller_.init())
	{
		return false;
	}

	//初始化Timer
	if (!timer_.init(&epoller_))
	{
		return false;
	}

	//初始化loop std::thread(function)创建一个新的线程并开始运行这个线程
	//[this] 通过引用捕获当前对象
	pLoop_ = new std::thread
	(
		[this]
		{
			threadIdx = tid_;                //设置线程的独立变量
			status_ = PRO_RUNNING;           //设置处理器状态为开始运行
			while (PRO_RUNNING == status_)
			{
				//清空所有列表
				if (actCoroutines_.size())
				{
					actCoroutines_.clear();  //清空活跃协程队列
				}
				if (timerExpiredCo_.size())
				{
					timerExpiredCo_.clear(); //清空超时协程队列
				}
				///@获取活跃事件（loop中唯一会阻塞的地方--用到了epoll_wait）
				epoller_.getActEvServ(parameter::epollTimeOutMs, actCoroutines_);

				///@（1）首先处理超时协程
				timer_.getExpiredCoroutines(timerExpiredCo_);
				size_t timerCoCnt = timerExpiredCo_.size();
				for (size_t i = 0; i < timerCoCnt; ++i)
				{
					resume(timerExpiredCo_[i]);                ///@唤醒这些超时的协程
				}

				///@（2）执行新来的协程
				Coroutine* pNewCo = nullptr;
				int runningQue = runningNewQue_;                    //0
				
				while (!newCoroutines_[runningQue].empty())    	    //当0队列非空时--执行新来的协程
				{
					{
						pNewCo = newCoroutines_[runningQue].front();
						newCoroutines_[runningQue].pop();
						coSet_.insert(pNewCo);
					}
					resume(pNewCo);                            //拿到新的协程并唤醒执行
				}

				{
					SpinlockGuard lock(newQueLock_);
					runningNewQue_ = !runningQue;              //1 这里是消费完（也就是加入所有的新来协程）就交换队列
				}

				///@（3）执行被唤醒的协程
				size_t actCoCnt = actCoroutines_.size();
				for (size_t i = 0; i < actCoCnt; ++i)
				{
					resume(actCoroutines_[i]);
				}

				///@（4）清除已经执行完毕的协程
				for (auto deadCo : removedCo_)
				{
					coSet_.erase(deadCo);
					//delete deadCo;
					{
						SpinlockGuard lock(coPoolLock_);   ///@注意这里的锁coPoolLock_,是与goNewCo形成竞争关系的
						coPool_.delete_obj(deadCo);
					}
				}
				removedCo_.clear();
				
			}
			status_ = PRO_STOPPED;
		}
    );
    return true;
}

//等待fd上的ev事件返回 实质上做的是将fd加入epoll监听
void Processor::waitEvent(int fd, int ev)
{
	epoller_.addEv(pCurCoroutine_, fd, ev);
	//让出CPU
	yield();
	epoller_.removeEv(pCurCoroutine_, fd, ev);
}

void Processor::stop()
{
	status_ = PRO_STOPPING;
}
//销毁该线程事件循环
void Processor::join()
{
	pLoop_->join();
}
//唤醒epoll
void Processor::wakeUpEpoller()
{
	timer_.wakeUp();
}
///@上层Scheduler调用的函数,创建一个新的协程
void Processor::goNewCo(std::function<void()>&& coFunc, size_t stackSize)
{
	//Coroutine* pCo = new Coroutine(this, stackSize, std::move(coFunc));
	Coroutine* pCo = nullptr;

	{
		SpinlockGuard lock(coPoolLock_);				///@在loop事件循环的最后一步竞争锁并执行
		pCo = coPool_.new_obj(this, stackSize, std::move(coFunc));	 //利用协程对象池创建一个协程
	}
	//运行该协程
	goCo(pCo);
}

void Processor::goNewCo(std::function<void()>& coFunc, size_t stackSize)
{
	//Coroutine* pCo = new Coroutine(this, stackSize, coFunc);
	Coroutine* pCo = nullptr;

	{
		SpinlockGuard lock(coPoolLock_);
		pCo = coPool_.new_obj(this, stackSize, coFunc);
	}
	goCo(pCo);
}

void Processor::killCurCo()
{
	removedCo_.push_back(pCurCoroutine_);
}
