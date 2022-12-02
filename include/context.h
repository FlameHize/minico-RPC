#pragma once
#include "utils.h"
#include "parameter.h"
#include <ucontext.h>
/**
 * 封装了ucontext上下文切换的一些操作
 */
namespace minico
{

    class Processor;
	class Context
	{
	public:
		Context(size_t stackSize);	
		~Context();

		Context(const Context& otherCtx) 
			: ctx_(otherCtx.ctx_), pStack_(otherCtx.pStack_)
		{ }

		Context(Context&& otherCtx)
			: ctx_(otherCtx.ctx_), pStack_(otherCtx.pStack_)
		{ }

		Context& operator=(const Context& otherCtx) = delete;

		/** 用函数指针设置当前context的上下文入口*/
		void makeContext(void (*func)(), Processor*, Context*);

		/** 保存当前程序的上下文*/
		void makeCurContext();

		/** 将当前上下文保存到oldCtx中，然后切换到当前上下文，若oldCtx为空，则直接运行*/
		void swapToMe(Context* pOldCtx);

		/** 获取当前上下文的ucontext_t指针*/
		inline ucontext_t* getUCtx() { return &ctx_; };

	private:
		/** 上下文结构*/
		ucontext_t ctx_;   	
		
		/** 栈指针*/
		void* pStack_;        
		
		/** 协程栈大小*/
		size_t stackSize_;        

	};

}
/**
 * @ucontext_t GNU提供的一组创建、保存、切换用户态执行上下文的API,使得用户在程序中保存当前的上下文成为可能,可以利用此来实现用户态线程,即协程
 * @typedef struct ucontext_t
 * {
 * 	struct ucontext_t* uc_link;
 * 	sigset_t uc_sigmask;
 * 	stack_t uc_stack;
 * 	mcontext_t uc_mcontext;
 * }
 * @uc_link:当前context执行结束后要执行的下一个context,如果uc_link为空,执行完当前context之后退出程序
 * @uc_sigmask:执行当前上下文过程中需要屏蔽的信号列表,即信号掩码
 * @uc_stack:当前context运行的栈信息
 * @uc_mcontext:保存具体的程序执行上下文,如PC值、堆栈指针以及寄存器等信息,实现依赖于底层,是平台硬件相关的
 * 
 * @typedef void makecontext(ucontext_t* ucp,void(void*)() func,int argc,...);
 * @函数描述:初始化一个ucontext_t,func参数指明了该context的入口函数,argc为入口参数的个数,每次参数的类型必须是int类型
 * @在makecontext之前,一般需要显式的初始化栈信息以及信号掩码,同时也需要初始化uc_link,以便于程序退出上下文后继续执行
 * @函数的作用是：修改ucp指向的上下文(从getcontext()调用中获取）
 * @(在调用makecontext()之前,调用者必须为此上下文分配一个新堆栈,并将其地址分配给ucp->uc堆栈,定义一个后续上下文,并将其地址分配给ucp->uc链接)
 * 
 * @！！！！！稍后激活此上下文时(使用setcontext()或swapcontext()),将调用函数func,并传递argc后面的一系列整数(int)参数；
 * 
 * @调用方必须在argc中指定这些参数的数目.当此函数返回时,后继上下文被激活.如果后续上下文指针为空,则线程退出
 * 
 * @typedef int swapcontext(ucontext_t* olducp,ucontext_t* newucp);
 * @函数描述:原子操作,保存当前上下文并将上下文切换到新的上下文运行
 * 
 * @typedef int getcontext(ucontext_t* ucp);
 * @函数描述:将当前的执行上下文保存在ucp中,以便于后续恢复上下文
 * 
 * @typedef int setcontext(const ucontext_t* ucp);
 * @函数描述:将当前程序切换到新的context,在执行正确的情况下该函数直接切换到新的执行状态,不会返回(到main)
 * 
 * 使用示例：
 *  #include <stdio.h>
    #include <ucontext.h>

    static ucontext_t ctx[3];

    static void func1(void)
    {
        // 切换到func2
        swapcontext(&ctx[1], &ctx[2]);

        // 返回后，切换到ctx[1].uc_link，也就是main的swapcontext返回处
    }
    static void func2(void)
    {
        // 切换到func1
        swapcontext(&ctx[2], &ctx[1]);

        // 返回后，切换到ctx[2].uc_link，也就是func1的swapcontext返回处
    }

    int main (void)
    {
        // 初始化context1，绑定函数func1和堆栈stack1
        char stack1[8192];
        getcontext(&ctx[1]);				        //将当前的执行上下文保存在ucp中,以便于后续恢复上下文
        ctx[1].uc_stack.ss_sp   = stack1;		    //指定该上下文结构体所对应的执行栈为stack1
        ctx[1].uc_stack.ss_size = sizeof(stack1);	//指定该协程栈的大小
        ctx[1].uc_link = &ctx[0];			        //指定该上下文结构体执行结束后要运行的下一个上下文结构体
        makecontext(&ctx[1], func1, 0);			    //指定ctx[1]上下文结构体新的上下文环境为func1函数

        // 初始化context2，绑定函数func2和堆栈stack2
        char stack2[8192];
        getcontext(&ctx[2]);
        ctx[2].uc_stack.ss_sp   = stack2;
        ctx[2].uc_stack.ss_size = sizeof(stack1);
        ctx[2].uc_link = &ctx[1];
        makecontext(&ctx[2], func2, 0);

        // 保存当前context，然后切换到context2上去，也就是func2
        swapcontext(&ctx[0], &ctx[2]);
        return 0;
    }
 */
