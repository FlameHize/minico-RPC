#include "../include/coroutine.h"
#include "../include/processor.h"
#include "../include/parameter.h"

using namespace minico;

static void coWrapFunc(Processor* pP)
{
	// 运行所属处理器当前协程
	pP->getCurRunningCo()->startFunc();
	// 执行后即销毁    
	pP->killCurCo();			
}

Coroutine::Coroutine(Processor* pMyProcessor, size_t stackSize, std::function<void()>&& func)
	: coFunc_(std::move(func)), pMyProcessor_(pMyProcessor), status_(CO_DEAD), ctx_(stackSize)
{
	status_ = CO_READY;            
}

Coroutine::Coroutine(Processor* pMyProcessor, size_t stackSize, std::function<void()>& func)
	: coFunc_(func), pMyProcessor_(pMyProcessor), status_(CO_DEAD), ctx_(stackSize)
{
	status_ = CO_READY;
}

Coroutine::~Coroutine()
{
}

void Coroutine::resume()
{
	// 每次唤醒此协程前，都需要获取该协程所在处理器的主上下文结构体
	Context* pMainCtx = pMyProcessor_->getMainCtx();   
	switch (status_)
	{
	// 首次运行
	case CO_READY:
		status_ = CO_RUNNING;
		// 设置当前协程被切出或运行完毕后自动回到处理器主循环
		ctx_.makeContext((void (*)(void)) coWrapFunc, pMyProcessor_, pMainCtx);
		// 保存处理器主循环程序的上下文，并将该协程切入CPU运行
		ctx_.swapToMe(pMainCtx);  
		break;
	// 让出CPU，恢复后再次运行
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
	status_ = CO_WAITING;      
};
