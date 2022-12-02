#pragma once
#include "utils.h"
#include <vector>
/**
 * 监视epoll中是否有事件发生 + 向epoll中更改fd
 */
struct epoll_event;

namespace minico
{
	class Coroutine;

	class Epoller
	{
	public:
		Epoller();
		~Epoller();

		DISALLOW_COPY_MOVE_AND_ASSIGN(Epoller);

		/** 初始化*/
		bool init();

		/** 修改要监听的事件*/
		bool modifyEv(Coroutine* pCo, int fd, int interesEv);

		/** 添加要监听的事件*/
		bool addEv(Coroutine* pCo, int fd, int interesEv);

		/** 移除被监听的事件*/
		bool removeEv(Coroutine* pCo, int fd, int interesEv);

		/** 获取激活的事件队列*/
		int getActEvServ(int timeOutMs, std::vector<Coroutine*>& activeEvServs);

	private:

		/** 判断该epoll控制器是否有效*/
		inline bool isEpollFdUseful() { return epollFd_ < 0 ? false : true; };

		/** 内部监听fd*/
		int epollFd_;

		/** 激活事件队列*/
		std::vector<struct epoll_event> activeEpollEvents_;
	};
}
