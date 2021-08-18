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

	//Socket�࣬������Socket����Ĭ�϶��Ƿ�������
	//ְ��
	//1���ṩfd���������API
	//2������fd����������
	//���������ü�������ĳһfdû�����˾ͻ�close
	class Socket
	{
	public:
		explicit Socket(int sockfd, std::string ip = "", int port = -1)
			: _sockfd(sockfd), _pRef(new int(1)), _port(port), _ip(std::move(ip))
		{
			if (sockfd > 0)
			{
				setNonBolckSocket();		//��������Ϊ������
			}
		}
		//Ĭ���Ƿ�������TCPЭ���socket
		//����������������fork�ӽ���ǰ��ĳ���ļ����ʱ��ָ���ã�������������fork�ӽ��̺�ִ��execʱ�͹رա�����ʵʱ�������ķ����ģ�����ν �� close-on-exec��
		//Ҫ�����ڴ���socket��ʱ�����SOCK_CLOEXEC��־�����ܹ��ﵽ����Ҫ���Ч������fork�ӽ�����ִ��exec��ʱ�򣬻������������̴�����socket
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

		//���ص�ǰSocket��fd
		int fd() const { return _sockfd; }

		//���ص�ǰSocket�Ƿ����
		bool isUseful() { return _sockfd >= 0; }

		//��ip��port����ǰSocket
		int bind(const char* ip,int port);

		//��ʼ������ǰSocket
		int listen();

		//����һ�����ӣ�����һ�������ӵ�Socket
		Socket accept();

		//��socket�ж�����
		ssize_t read(void* buf, size_t count);

		//ipʾ����"127.0.0.1"
		void connect(const char* ip, int port);

		//��socket��д����
		ssize_t send(const void* buf, size_t count);

		//��ȡ��ǰ�׽��ֵ�Ŀ��ip
		std::string ip() { return _ip; }

		//��ȡ��ǰ�׽��ֵ�Ŀ��port
		int port() { return _port; }

		//��ȡ�׽��ֵ�ѡ��,�ɹ��򷵻�true����֮������false
		bool getSocketOpt(struct tcp_info*) const;

		//��ȡ�׽��ֵ�ѡ����ַ���,�ɹ��򷵻�true����֮������false
		bool getSocketOptString(char* buf, int len) const;

		//��ȡ�׽��ֵ�ѡ����ַ���
		std::string getSocketOptString() const;

		//�ر��׽��ֵ�д����
		int shutdownWrite();

		//�����Ƿ���Nagle�㷨������Ҫ��������ݰ�����������ʱ���ܻ�����
		int setTcpNoDelay(bool on);

		//�����Ƿ��ַ����
		int setReuseAddr(bool on);

		//�����Ƿ�˿�����
		int setReusePort(bool on);

		//�����Ƿ�ʹ���������
		int setKeepAlive(bool on);

		//����socketΪ��������
		int setNonBolckSocket();

		//����socketΪ������
		int setBlockSocket();

		//void SetNoSigPipe();

	private:
		//����һ�����ӣ�����һ�������ӵ�Socket
		Socket accept_raw();

		//fd
		int _sockfd;

		//���ü���
		int* _pRef;

		//�˿ں�
		int _port;

		//ip
		std::string _ip;
	};

}
