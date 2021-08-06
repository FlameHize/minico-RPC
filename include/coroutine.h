//@author Liu Yukang
#pragma once
#include <functional>
#include "context.h"
#include "utils.h"
/**
 * netco����ݼ�����ĺ���������Ӧ���߳���������Э�̣�����ÿһ���̶߳�Ӧһ��Processorʵ��
 * Э��Coroutineʵ��������Processor����ѭ���ϣ�Processorʹ��Epoll��Timer�����������
 * ��Scheduler�򲢲�������ôһ��ѭ��������һ��ȫ�ֵ�������ĳ���߳��е���co_go()����һ����Э�̺�
 * ʵ�ʾͻ���ø�ʵ���ķ�����ѡ��һ��Э�����ٵ�Preocessor���ӹ����Э��
 */
namespace minico
{
	//Э�̵�����״̬ �ֱ�Ϊ0 1 2 3
	enum coStatus
	{
		CO_READY = 0,
		CO_RUNNING,
		CO_WAITING,
		CO_DEAD
	};

	class Processor;
	///@Э�����װ
	class Coroutine
	{
	public:
		Coroutine(Processor*, size_t stackSize, std::function<void()>&&);
		Coroutine(Processor*, size_t stackSize, std::function<void()>&);
		~Coroutine();

		DISALLOW_COPY_MOVE_AND_ASSIGN(Coroutine);

		//�ָ����е�ǰЭ��
		void resume();

		//��ͣ���е�ǰЭ��
		void yield();

		Processor* getMyProcessor(){return pMyProcessor_;}

		//���и�Э�̵ĺ���
		inline void startFunc() { coFunc_(); }

		//��ȡ��Э�̵�������
		inline Context* getCtx() { return &ctx_; }

	private:

		std::function<void()> coFunc_;		//��װ�Ĺ�������

		Processor* pMyProcessor_;		//��Ӧ�Ĵ���������

		int status_;				//Э�̵�״̬

		Context ctx_;				//Э�̵������Ľṹ

	};

}
