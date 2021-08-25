#pragma once

#include "utils.h"
#include "parameter.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <string>
#include <unistd.h>
#include <sys/socket.h>

#include "../include/logger.h"

struct tcp_info;

namespace minico 
{
	/** hook socket*/
	class Socket
	{
	public:
		explicit Socket(int sockfd, std::string ip = "", int port = -1)
			: _sockfd(sockfd), _pRef(new int(1)), _port(port), _ip(std::move(ip))
		{
			if (sockfd > 0)
			{
				setNonBolckSocket();		
			}
		}

		Socket()
			: _sockfd(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)),
			_pRef(new int(1)), _port(-1), _ip("")
		{ }

		Socket(const Socket& otherSock) : _sockfd(otherSock._sockfd)
		{
			*(otherSock._pRef) += 1;
			_pRef = otherSock._pRef;
			_ip = otherSock._ip;
			_port = otherSock._port;
		}

		Socket(Socket&& otherSock) : _sockfd(otherSock._sockfd)
		{
			*(otherSock._pRef) += 1;
			_pRef = otherSock._pRef;
			_ip = std::move(otherSock._ip);
			_port = otherSock._port;
		}

		Socket& operator=(const Socket& otherSock) = delete;

		~Socket();

		int fd() const { return _sockfd; }

		bool isUseful() { return _sockfd >= 0; }

		int bind(const char* ip,int port);

		int listen();

		Socket accept();

		ssize_t read(void* buf, size_t count);

		void connect(const char* ip, int port);

		ssize_t send(const void* buf, size_t count);

		std::string ip() { return _ip; }

		int port() { return _port; }

		bool getSocketOpt(struct tcp_info*) const;

		bool getSocketOptString(char* buf, int len) const;

		std::string getSocketOptString() const;

		int shutdownWrite();

		int setTcpNoDelay(bool on);

		int setReuseAddr(bool on);

		int setReusePort(bool on);

		int setKeepAlive(bool on);

		int setNonBolckSocket();

		int setBlockSocket();


	private:
		Socket accept_raw();

		int _sockfd;

		int* _pRef;

		int _port;

		std::string _ip;
	};

}
