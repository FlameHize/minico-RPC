#pragma once
#include <atomic>
#include "utils.h"

namespace minico {

	/** 配合std::atomic_int定义的二元信号量使用，为1表示资源可以使用，为0表示资源不可使用*/
	class Spinlock
	{
	public:
		Spinlock()
			: sem_(1)
		{ }

		~Spinlock() { unlock(); }

		DISALLOW_COPY_MOVE_AND_ASSIGN(Spinlock);

		void lock()
		{
			int exp = 1;
			//原子对象所封装的值与参数expected的物理内容相同,进行原子值的修改并返回true,否则返回false,不进行修改
			//bool compare_exchange_strong (T& expected, T val)
			//比较并交换被封装的值(strong)与参数expected所指定的值是否相等
			//相等,则用val替换原子对象的旧值
			//不相等,则用原子对象的旧值替换expected 
			//因此调用该函数之后，如果被该原子对象封装的值与参数 expected 所指定的值不相等，expected 中的内容就是原子对象的旧值
			while (!sem_.compare_exchange_strong(exp, 0))
			{
				exp = 1;
			}
		}

		void unlock()
		{
			/** std::atomic::store函数将类型为T的参数val 复制给原子对象所封装的值*/
			sem_.store(1);	
		}

	private:
		std::atomic_int sem_;
	};

}