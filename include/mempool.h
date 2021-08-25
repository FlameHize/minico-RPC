#pragma once
#include "parameter.h"
#include "utils.h"
#include "logger.h"

namespace minico
{
	/** 效仿STL二级空间配置器的实现*/
	struct MemBlockNode
	{
		union
		{
			MemBlockNode* next;
			char data;
		};
	};

	/**
	 * 每次可以从内存池中获取objSize大小的内存块 
	 * 该内存池块的大小是与对象大小强相关的
	 */
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

		void* AllocAMemBlock();
		void FreeAMemBlock(void* block);

	private:

		MemBlockNode* _freeListHead;

		MemBlockNode* _mallocListHead;

		size_t _mallocTimes;

		size_t objSize_;
	};

	template<size_t objSize>
	MemPool<objSize>::~MemPool()
	{
		while (_mallocListHead)
		{
			MemBlockNode* mallocNode = _mallocListHead;
			_mallocListHead = mallocNode->next;
			free(static_cast<void*>(mallocNode));	
		}
	}

	template<size_t objSize>
	void* MemPool<objSize>::AllocAMemBlock()
	{
		void* ret;
		if (nullptr == _freeListHead)
		{
			/** 首先会分配(40 + 分配次数) * 对象大小的内存空间*/
			size_t mallocCnt = parameter::memPoolMallocObjCnt + _mallocTimes;
			void* newMallocBlk = malloc(mallocCnt * objSize_ + sizeof(MemBlockNode));
			/** 将该内存空间的首地址解释为一个MemBlockNode指针*/
			MemBlockNode* mallocNode = static_cast<MemBlockNode*>(newMallocBlk);
			/** mallocNode是一个头节点*/
			mallocNode->next = _mallocListHead;

			_mallocListHead = mallocNode;

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
