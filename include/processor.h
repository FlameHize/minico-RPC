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
 * @brief ����������Ӧһ��CPU�ĺ��ģ���netco�ж�Ӧһ���̣߳� ��ʵ�ʾ���һ���߳�
 * @detail ������Э��Coroutineʵ����������������
 * @param[in] newCoroutines_ ��Э��˫������У�ʹ��һ�����������������Э�̣���һ�����и�Processor��ѭ������ִ��������Э�̣���������Ժ�ͽ������У�ÿ����һ����Э�̾ͻ���һ����ѭ����������ִ��������Э�̣�
 * @param[in] actCoroutines_ ��epoll�����Э�̶���.��epoll_wait������ʱ��Processor��ѭ���᳢�Դ�Epoll�л�ȡ��Ծ��Э�̣�����ڸ�actCoroutines�����У�Ȼ�����λָ�ִ��
 * @param[in] timerExpiredCo_ ��ʱ��Э�̶���.��epoll_wait������ʱ,Processor��ѭ�������ȳ��Դ�Timer�л�ȡ��Ծ��Э�̣������timerExpiredCo�����У�Ȼ�����λָ�ִ��
 * @param[in] removedCo_ ���Ƴ���Э�̶���.ִ�����Э�̻����ȷŵ��ö�����,��һ��ѭ�������ͳһ����
 * @detail ��ѭ��ִ��˳��timer->new->act->remove
 */
extern __thread int threadIdx;		//�߳����

namespace minico
{
	//�̵߳�״̬
	enum processerStatus
	{
		PRO_RUNNING = 0,
		PRO_STOPPING,
		PRO_STOPPED
	};
	//�Ƿ����ڼ�����Э��
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

		//����һ����Э�̣���Э�̵ĺ�����func
		void goNewCo(std::function<void()>&& func, size_t stackSize);
		void goNewCo(std::function<void()>& func, size_t stackSize);

		void yield();

		//��ǰЭ�̵ȴ�time����
		void wait(Time time);

		//�����ǰ�������е�Э��
		void killCurCo();
		//�̵߳��¼�ѭ��
		bool loop();
		//ֹͣ�߳�ѭ��
		void stop();

		void join();

		//�ȴ�fd�ϵ�ev�¼�����
		void waitEvent(int fd, int ev);

		//��ȡ��ǰ�������е�Э��
		inline Coroutine* getCurRunningCo() { return pCurCoroutine_; };

		inline Context* getMainCtx() { return &mainCtx_; }
		//��õ�ǰ�̴߳��ڵ�Э��������
		inline size_t getCoCnt() { return coSet_.size(); }
		//����һ��ָ����Э��
		void goCo(Coroutine* co);
		//����һ��ָ����Э��
		void goCoBatch(std::vector<Coroutine*>& cos);

	private:

		//�ָ�����ָ��Э��
		void resume(Coroutine*);
		//����epoll
		inline void wakeUpEpoller();

		//�ô��������̺߳�
		int tid_;
		//�̵߳�״̬
		int status_;
		//std�������̶߳���
		std::thread* pLoop_;

		//��������У�ʹ��˫�������(һ�����������������Э��,��һ�����и��߳��¼�ѭ������ִ��������Э��,�������ͽ�������)
		std::queue<Coroutine*> newCoroutines_[2];

		//������˫����������������еĶ��кţ���һ�������������
		volatile int runningNewQue_;

		Spinlock newQueLock_;

		Spinlock coPoolLock_;

		//std::mutex newCoQueMtx_;

		//EventEpoller���ֵĻ�Ծ�¼����ŵ��б�
		std::vector<Coroutine*> actCoroutines_;

		std::set<Coroutine*> coSet_;

		//��ʱ�������б�
		std::vector<Coroutine*> timerExpiredCo_;

		//���Ƴ���Э���б�Ҫ�Ƴ�ĳһ���¼����ȷ��ڸ��б��У�һ��ѭ�������Ż�����delete
		std::vector<Coroutine*> removedCo_;

		Epoller epoller_;                     //EPOLL������

		Timer timer_;                         //��ʱ��

		ObjPool<Coroutine> coPool_;           //�����

		Coroutine* pCurCoroutine_;            //��ǰЭ��

		Context mainCtx_;                     //���ڽ��е��ȱ����mainCTX�����Ľṹ��
	};

}

