#include "../include/socket.h"
#include "../include/scheduler.h"
#include "../../include/logger.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>  
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>

using namespace minico;

/** RAII*/
Socket::~Socket()
{
	--(*_pRef);
	if (!(*_pRef) && isUseful())
	{
		::close(_sockfd);
		delete _pRef;
	}
}

bool Socket::getSocketOpt(struct tcp_info* tcpi) const
{
	socklen_t len = sizeof(*tcpi);
	memset(tcpi, 0, sizeof(*tcpi));
	return ::getsockopt(_sockfd, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}

bool Socket::getSocketOptString(char* buf, int len) const
{
	struct tcp_info tcpi;
	bool ok = getSocketOpt(&tcpi);
	if (ok)
	{
		snprintf(buf, len, "unrecovered=%u "
			"rto=%u ato=%u snd_mss=%u rcv_mss=%u "
			"lost=%u retrans=%u rtt=%u rttvar=%u "
			"sshthresh=%u cwnd=%u total_retrans=%u",
			tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
			tcpi.tcpi_rto,          // Retransmit timeout in usec
			tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
			tcpi.tcpi_snd_mss,
			tcpi.tcpi_rcv_mss,
			tcpi.tcpi_lost,         // Lost packets
			tcpi.tcpi_retrans,      // Retransmitted packets out
			tcpi.tcpi_rtt,          // Smoothed round trip time in usec
			tcpi.tcpi_rttvar,       // Medium deviation
			tcpi.tcpi_snd_ssthresh,
			tcpi.tcpi_snd_cwnd,
			tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
	}
	return ok;
}

std::string Socket::getSocketOptString() const
{
	char buf[1024];
	buf[0] = '\0';
	getSocketOptString(buf, sizeof buf);
	return std::string(buf);
}


int Socket::bind(const char* ip,int port)
{
	_port = port;
	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(struct sockaddr_in));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
    if(ip == nullptr)
    {
        serv.sin_addr.s_addr = htonl(INADDR_ANY);   
    }
    else
    {
        serv.sin_addr.s_addr = inet_addr(ip);
    }
	int ret = ::bind(_sockfd, (struct sockaddr*) & serv, sizeof(serv));
	return ret;
}

int Socket::listen()
{
	int ret = ::listen(_sockfd, parameter::backLog);
	return ret;
}

Socket Socket::accept_raw()
{
	int connfd = -1;
	struct sockaddr_in client;
	socklen_t len = sizeof(client);
	connfd = ::accept(_sockfd, (struct sockaddr*) & client, &len);
	if (connfd < 0)
	{
		return Socket(connfd);
	}

	struct sockaddr_in* sock = (struct sockaddr_in*) & client;
	int port = ntohs(sock->sin_port);          
	struct in_addr in = sock->sin_addr;
	char ip[INET_ADDRSTRLEN];   
	inet_ntop(AF_INET, &in, ip, sizeof(ip));

	return Socket(connfd, std::string(ip), port);
}

Socket Socket::accept()
{
	auto ret(accept_raw());
	if(ret.isUseful())
	{
		return ret;
	}
	// 将该socket加入epoll监听池并切出所属的协程
	minico::Scheduler::getScheduler()->getProcessor(threadIdx)->waitEvent(_sockfd, EPOLLIN | EPOLLPRI | EPOLLRDHUP | EPOLLHUP);
	// 执行到此说明当前协程恢复运行 也就是加入epoll的fd存在激活的事件 那么就再连接一次
	auto con(accept_raw());
	if(con.isUseful())
	{
		return con;
	}
	// 加入： 失败的话 尝试重连
	return accept();
}

ssize_t Socket::read(void* buf, size_t count)
{
	// 调用系统接口读入数据到buf中
	auto ret = ::read(_sockfd, buf, count);
	//LOG_INFO("the read bytes len is %d",ret);

	if (ret >= 0)
	{
		// 一次读完 直接返回
		return ret;
	}
	/** 接收缓存区没有数据，则会返回-1*/
	if(ret == -1 && errno == EINTR)
	{
		// 出错 重读
		LOG_INFO("read has error");
		return read(buf, count);
	}
	// 还没有数据可读 将socket加入epoll监听池并切出所属的协程，等有数据两再回来读
	//LOG_INFO("the coroutine yield");
	minico::Scheduler::getScheduler()->getProcessor(threadIdx)->waitEvent(_sockfd, EPOLLIN | EPOLLPRI | EPOLLRDHUP | EPOLLHUP);
	//LOG_INFO("the coroutine wake");
	// Modify: 当前协程恢复运行 那么就继续读
	//return ::read(_sockfd, buf, count);
	return read(buf,count);
}

void Socket::connect(const char* ip, int port)
{
	struct sockaddr_in addr = {0};
	addr.sin_family= AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &addr.sin_addr);
	_ip = std::string(ip);
	_port = port;
	auto ret = ::connect(_sockfd, (struct sockaddr*)&addr, sizeof(sockaddr_in));
	if(ret == 0){
		return;
	}
	if(ret == -1 && errno == EINTR){
		return connect(ip, port);
	}
	minico::Scheduler::getScheduler()->getProcessor(threadIdx)->waitEvent(_sockfd, EPOLLOUT);
	return connect(ip, port);
}

ssize_t Socket::send(const void* buf, size_t count)
{
	size_t sendIdx = ::send(_sockfd, buf, count, MSG_NOSIGNAL);
	if (sendIdx >= count){
		return count;
	}
	minico::Scheduler::getScheduler()->getProcessor(threadIdx)->waitEvent(_sockfd, EPOLLOUT);
	return send((char *)buf + sendIdx, count - sendIdx);
}

int Socket::shutdownWrite()
{
	int ret = ::shutdown(_sockfd, SHUT_WR);
	return ret;
}

int Socket::setTcpNoDelay(bool on)
{
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(_sockfd, IPPROTO_TCP, TCP_NODELAY,
		&optval, static_cast<socklen_t>(sizeof optval));
	return ret;
}

int Socket::setReuseAddr(bool on)
{
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR,
		&optval, static_cast<socklen_t>(sizeof optval));
	return ret;
}

int Socket::setReusePort(bool on)
{
	int ret = -1;
#ifdef SO_REUSEPORT
	int optval = on ? 1 : 0;
	ret = ::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT,
		&optval, static_cast<socklen_t>(sizeof optval));
#endif
	return ret;
}

int Socket::setKeepAlive(bool on)
{
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(_sockfd, SOL_SOCKET, SO_KEEPALIVE,
		&optval, static_cast<socklen_t>(sizeof optval));
	return ret;
}

int Socket::setNonBolckSocket()
{
	auto flags = fcntl(_sockfd, F_GETFL, 0);
	int ret = fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK);  
	return ret;
}


int Socket::setBlockSocket()
{
	auto flags = fcntl(_sockfd, F_GETFL, 0);
	int ret = fcntl(_sockfd, F_SETFL, flags & ~O_NONBLOCK);   
	return ret;
}

ssize_t Socket::recvfrom(int sockfd, void* buf, int len, unsigned int flags,
						sockaddr* from, socklen_t* fromlen)
{
	if(sockfd != _sockfd)
	{
		LOG_INFO("ERROR: the sockfd is not same as current Socket");
		return -1;
	}
	// 调用系统接口读入数据到buf中
	auto ret = ::recvfrom(sockfd, buf, len, flags, from, fromlen);
	//LOG_INFO("the read bytes len is %d",ret);

	if (ret >= 0)
	{
		// 一次读完 直接返回
		return ret;
	}
	/** 接收缓存区没有数据，则会返回-1*/
	if(ret == -1 && errno == EINTR)
	{
		// 出错 重读
		LOG_INFO("recvfrom has error");
		return recvfrom(sockfd, buf, len, flags, from, fromlen);
	}
	// 还没有数据可读 将socket加入epoll监听池并切出所属的协程，等有数据两再回来读

	minico::Scheduler::getScheduler()->getProcessor(threadIdx)->waitEvent(sockfd, EPOLLIN | EPOLLPRI | EPOLLRDHUP | EPOLLHUP);

	// Modify: 当前协程恢复运行 那么就继续读
	return recvfrom(sockfd, buf, len, flags, from, fromlen);
}

ssize_t Socket::sendto(int sockfd, const void* buf, int len, unsigned int flags,
						const struct sockaddr* to, int tolen)
{
	if(sockfd != _sockfd)
	{
		LOG_INFO("ERROR: the sockfd is not same as current Socket");
		return -1;
	}
	size_t sendIdx = ::sendto(sockfd, buf, len, flags, to, tolen);
	if (sendIdx >= len){
		return len;
	}
	minico::Scheduler::getScheduler()->getProcessor(threadIdx)->waitEvent(sockfd, EPOLLOUT);
	return sendto(sockfd, (char*)buf + sendIdx, len - sendIdx, flags, to, tolen);
}