#include <iostream>
#include <sys/sysinfo.h>

#include "../include/processor.h"
#include "../include/minico_api.h"
#include "../include/socket.h"
#include "../include/mutex.h"

using namespace minico;

//minico http response with one acceptor test 
//ֻ��һ��acceptor�ķ���
void single_acceptor_server_test()
{
	//��Э���ϵ����lambda�հ�����std::function�������Э�̵������� ������ִ���������
	minico::co_go(
		[]{
			minico::Socket listener;
			if (listener.isUseful())
			{
				listener.setTcpNoDelay(true);
				listener.setReuseAddr(true);
				listener.setReusePort(true);
				if (listener.bind(8099) < 0)
				{
					return;
				}
				listener.listen();
			}
			//���������һ�����޵�ѭ��
			while (1)
			{
				minico::Socket* conn = new minico::Socket(listener.accept());
				conn->setTcpNoDelay(true);
				//ÿ��accept�ɹ���֮��,�ʹ���һ��Э�̽��й���
				minico::co_go(
					[conn]
					{
						std::vector<char> buf;
						buf.resize(2048);
						while (1)
						{
							auto readNum = conn->read((void*)&(buf[0]), buf.size());	//����Ҳ�ᷢ��Э���л�(�л�ȥ������loop)
							std::string ok = "HTTP/1.0 200 OK\r\nServer: minico/0.1.0\r\nContent-Type: text/html\r\n\r\n";
							if(readNum < 0)
							{
								break;
							}
							conn->send(ok.c_str(), ok.size());
							conn->send((void*)&(buf[0]), readNum);
							if(readNum < (int)buf.size())
							{
								break;
							}
						}
						minico::co_sleep(100);//��Ҫ��һ�£�����û������Ͼ͹ر���
						delete conn;
					}
					);
			}
		}
	);
}

//minico http response with multi acceptor test 
//ÿ���߳�һ��acceptor�ķ���
void multi_acceptor_server_test()
{
	auto tCnt = ::get_nprocs_conf();
	for (int i = 0; i < tCnt; ++i)
	{
		minico::co_go(
			[]
			{
				minico::Socket listener;
				if (listener.isUseful())
				{
					listener.setTcpNoDelay(true);
					listener.setReuseAddr(true);
					listener.setReusePort(true);
					if (listener.bind(8099) < 0)
					{
						return;
					}
					listener.listen();
				}
				while (1)
				{
					minico::Socket* conn = new minico::Socket(listener.accept());
					conn->setTcpNoDelay(true);
					minico::co_go(
						[conn]
						{
							std::string hello("HTTP/1.0 200 OK\r\nServer: minico/0.1.0\r\nContent-Length: 72\r\nContent-Type: text/html\r\n\r\n<HTML><TITLE>hello</TITLE>\r\n<BODY><P>hello word!\r\n</BODY></HTML>\r\n");
							//std::string hello("<HTML><TITLE>hello</TITLE>\r\n<BODY><P>hello word!\r\n</BODY></HTML>\r\n");
							char buf[1024];
							if (conn->read((void*)buf, 1024) > 0)
							{
								conn->send(hello.c_str(), hello.size());
								minico::co_sleep(50);//��Ҫ��һ�£�����û������Ͼ͹ر���
							}
							delete conn;
						}
						);
				}
			}
			,parameter::coroutineStackSize, i);
	}
	
}

//��Ϊ�ͻ��˵Ĳ��ԣ����������server����
void client_test(){
	minico::co_go(
				[]
				{
					char buf[1024];
					while(1){
						minico::co_sleep(2000);
						minico::Socket s;
						s.connect("127.0.0.1", 8099);
						s.send("ping", 4);
						s.read(buf, 1024);
						std::cout << std::string(buf) << std::endl;
					}
				}
				);
}

//��д������
void mutex_test(minico::RWMutex& mu){
	for(int i = 0; i < 10; ++i)
	if(i < 5){
		minico::co_go(
		[&mu, i]{
			mu.rlock();
			std::cout << i << " : start reading" << std::endl;
			minico::co_sleep(100);
			std::cout << i << " : finish reading" << std::endl;
			mu.runlock();
			mu.wlock();
			std::cout << i << " : start writing" << std::endl;
			minico::co_sleep(100);
			std::cout << i << " : finish writing" << std::endl;
			mu.wunlock();
		}
		);
	}else{
		minico::co_go(
		[&mu, i]{
			mu.wlock();
			std::cout << i << " : start writing" << std::endl;
			minico::co_sleep(100);
			std::cout << i << " : finish writing" << std::endl;
			mu.wunlock();
			mu.rlock();
			std::cout << i << " : start reading" << std::endl;
			minico::co_sleep(100);
			std::cout << i << " : finish reading" << std::endl;
			mu.runlock();
		}
	);
	}
	
}

int main()
{
	//netco::RWMutex mu;
	//mutex_test(mu);
	single_acceptor_server_test();
	//multi_acceptor_server_test();
	//client_test();
	//join�������� �ȴ����е��߲���� ���ܽ������߲㣨Ҳ���ǵ�ǰ��main���̣�
	minico::sche_join();
	std::cout << "end" << std::endl;
	return 0;
}
