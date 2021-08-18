#pragma once
#include <stddef.h>

namespace minico
{
	namespace parameter
	{
		//Э��ջ��С
		const static size_t coroutineStackSize = 8 * 1024;

		//��ȡ��Ծ��epoll_event������ĳ�ʼ����
		static constexpr int epollerListFirstSize = 16;

		//epoll_wait������ʱ��
		static constexpr int epollTimeOutMs = -1;

		//�������еĳ���
		constexpr static unsigned backLog = 4096;

		//�ڴ��û�п����ڴ��ʱ����memPoolMallocObjCnt�������С���ڴ��
		static constexpr size_t memPoolMallocObjCnt = 40;
	}
	
}