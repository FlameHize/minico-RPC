#pragma once
#include "parameter.h"
#include "utils.h"

namespace minico
{
	//Ч��STL�����ռ���������ʵ��
	struct MemBlockNode
	{
		union
		{
			MemBlockNode* next;
			char data;
		};
	};

	//ÿ�ο��Դ��ڴ���л�ȡobjSize��С���ڴ�� �ڴ�ص�ʵ�֣����ڴ�ؿ�Ĵ�С��������Сǿ��صģ�
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
		//�����ڴ��
		void* AllocAMemBlock();
		void FreeAMemBlock(void* block);

	private:
		///@��������
		MemBlockNode* _freeListHead;
		///@malloc�Ĵ��ڴ������,����ʽ���ŵ�malloc�������ڴ���ͷ����ָ�룩�����������еĿ�������
		MemBlockNode* _mallocListHead;
		///@ʵ��malloc�Ĵ���
		size_t _mallocTimes;
		///@ÿ���ڴ���С
		size_t objSize_;
	};

	template<size_t objSize>
	MemPool<objSize>::~MemPool()
	{
		while (_mallocListHead)
		{
			MemBlockNode* mallocNode = _mallocListHead;
			_mallocListHead = mallocNode->next;
			free(static_cast<void*>(mallocNode));	//�൱���ջ����е��ڴ�
		}
	}

	template<size_t objSize>
	void* MemPool<objSize>::AllocAMemBlock()
	{
		void* ret;
		//�����������Ľڵ�����Ϊ�� �͸���һ��_mallocListHead��ά��һ��_freeListHead
		if (nullptr == _freeListHead)
		{
			//���Ȼ����(40 + �������) * �����С���ڴ�ռ�
			size_t mallocCnt = parameter::memPoolMallocObjCnt + _mallocTimes;
			void* newMallocBlk = malloc(mallocCnt * objSize_ + sizeof(MemBlockNode));
			//�����ڴ�ռ���׵�ַ����Ϊһ��MemBlockNodeָ��
			MemBlockNode* mallocNode = static_cast<MemBlockNode*>(newMallocBlk);
			//�൱��mallocNode��һ��ͷ�ڵ�
			mallocNode->next = _mallocListHead;
			//_mallocListHead��ά����һ���ڵ�ָ��,��nextָ��������(ͬ��Ҳ��ָ������ڴ�ռ�����)
			_mallocListHead = mallocNode;
			//���ÿ��ڴ�ռ�ת��Ϊchar*������
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
	//�ͷ�,�൱�ڹ黹������������ȥ
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
