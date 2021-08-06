#include "../include/coroutine.h"
#include "../include/processor.h"
#include "../include/parameter.h"

using namespace minico;

static void coWrapFunc(Processor* pP)
{
	pP->getCurRunningCo()->startFunc();    ///@获取当前处理器保存的协程并开始执行
	pP->killCurCo();			//协程执行完即销毁
}

///@创建一个协程所需要的参数：对应的线程、协程栈大小、协程的工作函数（这也是最正规的封装方式）
Coroutine::Coroutine(Processor* pMyProcessor, size_t stackSize, std::function<void()>&& func)
	: coFunc_(std::move(func)), pMyProcessor_(pMyProcessor), status_(CO_DEAD), ctx_(stackSize)
{
	status_ = CO_READY;            //初始化为ready状态
}

Coroutine::Coroutine(Processor* pMyProcessor, size_t stackSize, std::function<void()>& func)
	: coFunc_(func), pMyProcessor_(pMyProcessor), status_(CO_DEAD), ctx_(stackSize)
{
	status_ = CO_READY;
}

Coroutine::~Coroutine()
{
}

///@唤醒该协程（将当前的寄存器状态保存到pMyProcessor中的mainctx上下文结构中）
void Coroutine::resume()
{
	Context* pMainCtx = pMyProcessor_->getMainCtx();   ///@获取该处理线程的主上下文结构体（用于调度）
	//根据当前协程的状态进行选择
	switch (status_)
	{
	//协程第一次开始执行 状态就是CO_READY
	case CO_READY:
		//设置协程的状态为开始运行
		status_ = CO_RUNNING;
		///@设置下一要执行的上下文环境为pMainCtx（也就是处理器保存的ctx,换言之,这里是自动回到处理器）
		ctx_.makeContext((void (*)(void)) coWrapFunc, pMyProcessor_, pMainCtx);
		///@将当前上下文环境保存到pMainCtx的ctx结构体中,并使用当前协程的Context.ctx切换上下文信息到当前CPU中
		ctx_.swapToMe(pMainCtx);  
		break;
	//协程执行到一半被切换出CPU 状态就是CO_WAITING
	case CO_WAITING:
		status_ = CO_RUNNING;
		ctx_.swapToMe(pMainCtx);
		break;
	default:
		break;
	}
}

void Coroutine::yield()
{
	status_ = CO_WAITING;      //只是简单的更改了状态
};
