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
	mainCtx_.makeCurContext();         //����ǰ��ִ�������ı��浽mainCtx��ucontext�ṹ����
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

///@����һ��ָ����Э�̣���loop�е��ã�
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
	///@���̱߳���ĵ�ǰЭ��ָ��ָ���Э��
	pCurCoroutine_ = pCo;
	///@ʵ��ִ�л��Ѹ�Э�̣����ж��ֳ���Ϣ���浽mainCtx�У�,ִ����ú���֮������������ת�Ƶ�Э�̵Ĺ�������startFunc()��
	pCo->resume();
}

///@��ͣ���е�ǰЭ��
void Processor::yield()
{
	///@���õ�ǰִ�е�Э�̵�״̬ΪCO_WAITING
	pCurCoroutine_->yield();                       
	//����ǰִ�е�Э���ж���Ϣ���浽��ctx�����Ľṹ���У�������mainCtxˢ�¼Ĵ���
	mainCtx_.swapToMe(pCurCoroutine_->getCtx());
}
///@��ǰЭ�̵ȴ�time(ms)
void Processor::wait(Time time)
{
	pCurCoroutine_->yield();			//��ͣ��Э�̣�ʵ��ֻ�Ǹ�����Э�̵�״̬ΪCO_WAITING��
	timer_.runAfter(time,pCurCoroutine_);		//���붨ʱ����
	mainCtx_.swapToMe(pCurCoroutine_->getCtx());	///@����ǰЭ�̴�CPU���г�,ʹ�̱߳���������Ľṹ����mainCtx_ռ��CPU����ʼִ��
}
///@��ʼ����һ��ָ����Э��pCo
void Processor::goCo(Coroutine* pCo)
{
	{
		SpinlockGuard lock(newQueLock_);	///@������Ƕ�����,�����Ǵ��������Э��,����ط����߼����ص�,Ҳ�����������еĺ���
		newCoroutines_[!runningNewQue_].push(pCo);	///@�ص㣡������Э�̷����߳��д洢,֮���Ѳ�����
	}
	///@ÿ����һ����Э�̾ͻỽ��һ��Processor��ѭ��,������ִ��������Э��(��Ϊepoll_wait��������,������ܼ�����û��ִ��)
	wakeUpEpoller();
}
//����һ��ָ����Э��cos
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

//�߳��¼�ѭ��
bool Processor::loop()
{
	//��ʼ��Epoller
	if (!epoller_.init())
	{
		return false;
	}

	//��ʼ��Timer
	if (!timer_.init(&epoller_))
	{
		return false;
	}

	//��ʼ��loop std::thread(function)����һ���µ��̲߳���ʼ��������߳�
	//[this] ͨ�����ò���ǰ����
	pLoop_ = new std::thread
	(
		[this]
		{
			threadIdx = tid_;                //�����̵߳Ķ�������
			status_ = PRO_RUNNING;           //���ô�����״̬Ϊ��ʼ����
			while (PRO_RUNNING == status_)
			{
				//��������б�
				if (actCoroutines_.size())
				{
					actCoroutines_.clear();  //��ջ�ԾЭ�̶���
				}
				if (timerExpiredCo_.size())
				{
					timerExpiredCo_.clear(); //��ճ�ʱЭ�̶���
				}
				///@��ȡ��Ծ�¼���loop��Ψһ�������ĵط�--�õ���epoll_wait��
				epoller_.getActEvServ(parameter::epollTimeOutMs, actCoroutines_);

				///@��1�����ȴ���ʱЭ��
				timer_.getExpiredCoroutines(timerExpiredCo_);
				size_t timerCoCnt = timerExpiredCo_.size();
				for (size_t i = 0; i < timerCoCnt; ++i)
				{
					resume(timerExpiredCo_[i]);                ///@������Щ��ʱ��Э��
				}

				///@��2��ִ��������Э��
				Coroutine* pNewCo = nullptr;
				int runningQue = runningNewQue_;                    //0
				
				while (!newCoroutines_[runningQue].empty())    	    //��0���зǿ�ʱ--ִ��������Э��
				{
					{
						pNewCo = newCoroutines_[runningQue].front();
						newCoroutines_[runningQue].pop();
						coSet_.insert(pNewCo);
					}
					resume(pNewCo);                            //�õ��µ�Э�̲�����ִ��
				}

				{
					SpinlockGuard lock(newQueLock_);
					runningNewQue_ = !runningQue;              //1 �����������꣨Ҳ���Ǽ������е�����Э�̣��ͽ�������
				}

				///@��3��ִ�б����ѵ�Э��
				size_t actCoCnt = actCoroutines_.size();
				for (size_t i = 0; i < actCoCnt; ++i)
				{
					resume(actCoroutines_[i]);
				}

				///@��4������Ѿ�ִ����ϵ�Э��
				for (auto deadCo : removedCo_)
				{
					coSet_.erase(deadCo);
					//delete deadCo;
					{
						SpinlockGuard lock(coPoolLock_);   ///@ע���������coPoolLock_,����goNewCo�γɾ�����ϵ��
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

//�ȴ�fd�ϵ�ev�¼����� ʵ���������ǽ�fd����epoll����
void Processor::waitEvent(int fd, int ev)
{
	epoller_.addEv(pCurCoroutine_, fd, ev);
	//�ó�CPU
	yield();
	epoller_.removeEv(pCurCoroutine_, fd, ev);
}

void Processor::stop()
{
	status_ = PRO_STOPPING;
}
//���ٸ��߳��¼�ѭ��
void Processor::join()
{
	pLoop_->join();
}
//����epoll
void Processor::wakeUpEpoller()
{
	timer_.wakeUp();
}
///@�ϲ�Scheduler���õĺ���,����һ���µ�Э��
void Processor::goNewCo(std::function<void()>&& coFunc, size_t stackSize)
{
	//Coroutine* pCo = new Coroutine(this, stackSize, std::move(coFunc));
	Coroutine* pCo = nullptr;

	{
		SpinlockGuard lock(coPoolLock_);				///@��loop�¼�ѭ�������һ����������ִ��
		pCo = coPool_.new_obj(this, stackSize, std::move(coFunc));	 //����Э�̶���ش���һ��Э��
	}
	//���и�Э��
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
