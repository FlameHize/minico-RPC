#pragma once
#include <type_traits>
#include "mempool.h"
/**
 * @brief 为用户所使用，在库中主要用在Coroutine实例的创建上
 * 对象池创建对象时，首先会从内存池中取出相应大小的块，内存池是与对象大小相关的，
 * 其中有一个空闲链表，每次分配空间都从空闲链表上取
 * 若空闲链表没有内容时，首先会分配（40 + 分配次数）* 对象大小的空间，然后分成一个个块挂在空闲链表上
 * 这里空闲链表节点没有使用额外的空间：效仿的stl的二级配置器中的方法，将数据和next指针放在了一个union中
 * 从内存池取出所需内存块后，会判断对象是否拥有non-trivial构造函数，没有的话直接返回，有的话使用placement new构造对象
 */
namespace minico
{
	//对象池的实现
	template<class T>
	class ObjPool
	{
	public:
		ObjPool() {};
		~ObjPool() {};

		DISALLOW_COPY_MOVE_AND_ASSIGN(ObjPool);
		//创建一个对象
		template<typename... Args>
		inline T* new_obj(Args... args);
		//删除一个对象
		inline void delete_obj(void* obj);

	private:
		//true_type与false_type用来判断是否需要重要的析构函数
		template<typename... Args>
		inline T* new_aux(std::true_type, Args... args);

		template<typename... Args>
		inline T* new_aux(std::false_type, Args... args);

		inline void delete_aux(std::true_type, void* obj);

		inline void delete_aux(std::false_type, void* obj);
		//自带的内存池
		MemPool<sizeof(T)> _memPool;

	};

	template<class T>
	template<typename... Args>
	inline T* ObjPool<T>::new_obj(Args... args)
	{
		//typedef std::integral_constant<bool,true> std::true_type 
		//typedef std::integral_constant<bool,false> std::false_type
		return new_aux(std::integral_constant<bool, std::is_trivially_constructible<T>::value>(), args...);
	}

	template<class T>
	template<typename... Args>
	inline T* ObjPool<T>::new_aux(std::true_type, Args... args)
	{
		return static_cast<T*>(_memPool.AllocAMemBlock());
	}

	template<class T>
	template<typename... Args>
	inline T* ObjPool<T>::new_aux(std::false_type, Args... args)
	{
		void* newPos = _memPool.AllocAMemBlock();
		//placement new版本，它本质上是对operator new的重载，定义于#include <new>中。它不分配内存，
		//调用合适的构造函数在ptr所指的地方构造一个对象，之后返回实参指针ptr
		//调用方式 new(p)A();
		//new (p)A()调用placement new之后，还会在p上调用A::A()，这里的p可以是堆中动态分配的内存，也可以是栈中缓冲
		return new(newPos) T(args...);
	}

	template<class T>
	inline void ObjPool<T>::delete_obj(void* obj)
	{
		if (!obj)
		{
			return;
		}
		delete_aux(std::integral_constant<bool, std::is_trivially_destructible<T>::value>(), obj);
	}

	template<class T>
	inline void ObjPool<T>::delete_aux(std::true_type, void* obj)
	{
		_memPool.FreeAMemBlock(obj);
	}

	template<class T>
	inline void ObjPool<T>::delete_aux(std::false_type, void* obj)
	{
		(static_cast<T*>(obj))->~T();
		_memPool.FreeAMemBlock(obj);
	}

}
