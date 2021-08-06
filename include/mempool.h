#pragma once
#include "parameter.h"
#include "utils.h"

namespace minico
{
	//效仿STL二级空间配置器的实现
	struct MemBlockNode
	{
		union
		{
			MemBlockNode* next;
			char data;
		};
	};

	//每次可以从内存池中获取objSize大小的内存块 内存池的实现（该内存池块的大小是与对象大小强相关的）
	template<size_t objSize>
	class MemPool
	{
	public:
		MemPool()
			:_freeListHead(nullptr), _mallocListHead(nullptr), _mallocTimes(0)
		{
			if (objSize < sizeof(MemBlockNode))
			{
				objSize_ = sizeof(MemBlockNode);
			}
			else
			{
				objSize_ = objSize;
			}
		};

		~MemPool();

		DISALLOW_COPY_MOVE_AND_ASSIGN(MemPool);
		//分配内存块
		void* AllocAMemBlock();
		void FreeAMemBlock(void* block);

	private:
		///@空闲链表
		MemBlockNode* _freeListHead;
		///@malloc的大内存块链表,（形式：放到malloc出来的内存块的头部的指针）用来连接所有的空闲链表
		MemBlockNode* _mallocListHead;
		///@实际malloc的次数
		size_t _mallocTimes;
		///@每个内存块大小
		size_t objSize_;
	};

	template<size_t objSize>
	MemPool<objSize>::~MemPool()
	{
		while (_mallocListHead)
		{
			MemBlockNode* mallocNode = _mallocListHead;
			_mallocListHead = mallocNode->next;
			free(static_cast<void*>(mallocNode));	//相当于收回所有的内存
		}
	}

	template<size_t objSize>
	void* MemPool<objSize>::AllocAMemBlock()
	{
		void* ret;
		//如果空闲链表的节点数量为空 就更新一个_mallocListHead和维护一条_freeListHead
		if (nullptr == _freeListHead)
		{
			//首先会分配(40 + 分配次数) * 对象大小的内存空间
			size_t mallocCnt = parameter::memPoolMallocObjCnt + _mallocTimes;
			void* newMallocBlk = malloc(mallocCnt * objSize_ + sizeof(MemBlockNode));
			//将该内存空间的首地址解释为一个MemBlockNode指针
			MemBlockNode* mallocNode = static_cast<MemBlockNode*>(newMallocBlk);
			//相当于mallocNode是一个头节点
			mallocNode->next = _mallocListHead;
			//_mallocListHead是维护的一个节点指针,其next指向其自身(同样也是指向这块内存空间的起点)
			_mallocListHead = mallocNode;
			//将该块内存空间转化为char*来解释
			newMallocBlk = static_cast<char*>(newMallocBlk) + sizeof(MemBlockNode);
			for (size_t i = 0; i < mallocCnt; ++i)
			{
				MemBlockNode* newNode = static_cast<MemBlockNode*>(newMallocBlk);
				newNode->next = _freeListHead;
				_freeListHead = newNode;
				newMallocBlk = static_cast<char*>(newMallocBlk) + objSize_;
			}
			++_mallocTimes;
		}
		ret = &(_freeListHead->data);
		_freeListHead = _freeListHead->next;
		return ret;
	}
	//释放,相当于归还到空闲链表中去
	template<size_t objSize>
	void MemPool<objSize>::FreeAMemBlock(void* block)
	{
		if (nullptr == block)
		{
			return;
		}
		MemBlockNode* newNode = static_cast<MemBlockNode*>(block);
		newNode->next = _freeListHead;
		_freeListHead = newNode;
	}
}
