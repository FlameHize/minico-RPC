#pragma once
#include "utils.h"
#include "parameter.h"
#include <ucontext.h>
/**
 * ��װ��ucontext�������л���һЩ����
 */
namespace minico
{

    class Processor;
	class Context
	{
	public:
		Context(size_t stackSize);	//ָ����Э�̵�ջ��С
		~Context();

		Context(const Context& otherCtx) 
			: ctx_(otherCtx.ctx_), pStack_(otherCtx.pStack_)
		{ }

		Context(Context&& otherCtx)
			: ctx_(otherCtx.ctx_), pStack_(otherCtx.pStack_)
		{ }

		Context& operator=(const Context& otherCtx) = delete;

		//�ú���ָ�����õ�ǰcontext�����������
		void makeContext(void (*func)(), Processor*, Context*);

		//ֱ���õ�ǰ����״̬���õ�ǰcontext��������
		void makeCurContext();

		//����ǰ�����ı��浽oldCtx�У�Ȼ���л�����ǰ�����ģ���oldCtxΪ�գ���ֱ������
		void swapToMe(Context* pOldCtx);

		//��ȡ��ǰ�����ĵ�ucontext_tָ��
		inline ucontext_t* getUCtx() { return &ctx_; };

	private:

		ucontext_t ctx_;   //�����Ľṹ

		void* pStack_;            //ջָ��

		size_t stackSize_;        //ջ��С

	};

}
/**
 * @ucontext_t GNU�ṩ��һ�鴴�������桢�л��û�ִ̬�������ĵ�API,ʹ���û��ڳ����б��浱ǰ�������ĳ�Ϊ����,�������ô���ʵ���û�̬�߳�,��Э��
 * @typedef struct ucontext_t
 * {
 * 	struct ucontext_t* uc_link;
 * 	sigset_t uc_sigmask;
 * 	stack_t uc_stack;
 * 	mcontext_t uc_mcontext;
 * }
 * @uc_link:��ǰcontextִ�н�����Ҫִ�е���һ��context,���uc_linkΪ��,ִ���굱ǰcontext֮���˳�����
 * @uc_sigmask:ִ�е�ǰ�����Ĺ�������Ҫ���ε��ź��б�,���ź�����
 * @uc_stack:��ǰcontext���е�ջ��Ϣ
 * @uc_mcontext:�������ĳ���ִ��������,��PCֵ����ջָ���Լ��Ĵ�������Ϣ,ʵ�������ڵײ�,��ƽ̨Ӳ����ص�
 * 
 * @typedef void makecontext(ucontext_t* ucp,void(void*)() func,int argc,...);
 * @��������:��ʼ��һ��ucontext_t,func����ָ���˸�context����ں���,argcΪ��ڲ����ĸ���,ÿ�β��������ͱ�����int����
 * @��makecontext֮ǰ,һ����Ҫ��ʽ�ĳ�ʼ��ջ��Ϣ�Լ��ź�����,ͬʱҲ��Ҫ��ʼ��uc_link,�Ա��ڳ����˳������ĺ����ִ��
 * @�����������ǣ��޸�ucpָ���������(��getcontext()�����л�ȡ��
 * @(�ڵ���makecontext()֮ǰ,�����߱���Ϊ�������ķ���һ���¶�ջ,�������ַ�����ucp->uc��ջ,����һ������������,�������ַ�����ucp->uc����)
 * 
 * @�����������Ժ󼤻��������ʱ(ʹ��setcontext()��swapcontext()),�����ú���func,������argc�����һϵ������(int)������
 * 
 * @���÷�������argc��ָ����Щ��������Ŀ.���˺�������ʱ,��������ı�����.�������������ָ��Ϊ��,���߳��˳�
 * 
 * @typedef int swapcontext(ucontext_t* olducp,ucontext_t* newucp);
 * @��������:ԭ�Ӳ���,���浱ǰ�����Ĳ����������л����µ�����������
 * 
 * @typedef int getcontext(ucontext_t* ucp);
 * @��������:����ǰ��ִ�������ı�����ucp��,�Ա��ں����ָ�������
 * 
 * @typedef int setcontext(const ucontext_t* ucp);
 * @��������:����ǰ�����л����µ�context,��ִ����ȷ������¸ú���ֱ���л����µ�ִ��״̬,���᷵��(��main)
 * 
 * ʹ��ʾ����
 *  #include <stdio.h>
    #include <ucontext.h>

    static ucontext_t ctx[3];

    static void func1(void)
    {
        // �л���func2
        swapcontext(&ctx[1], &ctx[2]);

        // ���غ��л���ctx[1].uc_link��Ҳ����main��swapcontext���ش�
    }
    static void func2(void)
    {
        // �л���func1
        swapcontext(&ctx[2], &ctx[1]);

        // ���غ��л���ctx[2].uc_link��Ҳ����func1��swapcontext���ش�
    }

    int main (void)
    {
        // ��ʼ��context1���󶨺���func1�Ͷ�ջstack1
        char stack1[8192];
        getcontext(&ctx[1]);				//����ǰ��ִ�������ı�����ucp��,�Ա��ں����ָ�������
        ctx[1].uc_stack.ss_sp   = stack1;		//ָ���������Ľṹ������Ӧ��ִ��ջΪstack1
        ctx[1].uc_stack.ss_size = sizeof(stack1);	//ָ����Э��ջ�Ĵ�С
        ctx[1].uc_link = &ctx[0];			//ָ���������Ľṹ��ִ�н�����Ҫ���е���һ�������Ľṹ��
        makecontext(&ctx[1], func1, 0);			//ָ��ctx[1]�����Ľṹ���µ������Ļ���Ϊfunc1����

        // ��ʼ��context2���󶨺���func2�Ͷ�ջstack2
        char stack2[8192];
        getcontext(&ctx[2]);
        ctx[2].uc_stack.ss_sp   = stack2;
        ctx[2].uc_stack.ss_size = sizeof(stack1);
        ctx[2].uc_link = &ctx[1];
        makecontext(&ctx[2], func2, 0);

        // ���浱ǰcontext��Ȼ���л���context2��ȥ��Ҳ����func2
        swapcontext(&ctx[0], &ctx[2]);
        return 0;
    }
    @�ص�������ݣ�����makecontext����Ҫ�Ĺ����������ú���ָ��Ͷ�ջ����Ӧcontext�����sp��pc�Ĵ����У���Ҳ����Ϊʲômakecontext����ǰ������Ҫ��getcontext�µ�ԭ��
 */
