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

		bool init();

		bool modifyEv(Coroutine* pCo, int fd, int interesEv);

		bool addEv(Coroutine* pCo, int fd, int interesEv);

		bool removeEv(Coroutine* pCo, int fd, int interesEv);

		int getActEvServ(int timeOutMs, std::vector<Coroutine*>& activeEvServs);

	private:

		inline bool isEpollFdUseful() { return epollFd_ < 0 ? false : true; };

		int epollFd_;

		std::vector<struct epoll_event> activeEpollEvents_;
	};
}
