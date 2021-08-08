#include <iostream>
#include <sys/sysinfo.h>

#include "../include/processor.h"
#include "../include/minico_api.h"
#include "../include/socket.h"
#include "../include/mutex.h"
#include "../include/logger.h"
#include "../include/tcp_server.h"

using namespace minico;


//minico http response with multi acceptor test 
//每条线程一个acceptor的服务
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
					if (listener.bind(nullptr,8099) < 0)
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
								minico::co_sleep(50);//需要等一下，否则还没发送完毕就关闭了
							}
							delete conn;
						}
						);
				}
			}
			,parameter::coroutineStackSize, i);
	}
	
}

//读写锁测试
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

/** 所有的客户端程序必须在一个协程中运行*/
void client_test()
{
    minico::co_go([](){
        while(true)
        {
            Client client;
            client.connect("127,0,0,1",12345);
            char buf[1024];
            client.send("ping", 4);
            client.recv(buf, 1024);
            std::cout << std::string(buf) << std::endl;
        }
         
    });
}

int main()
{
	//netco::RWMutex mu;
	//mutex_test(mu);
	//single_acceptor_server_test();
	//multi_acceptor_server_test();
	//client_test();
	//join用于阻塞 等待所有的线层结束 才能结束主线层（也就是当前的main进程）
    LOG_INFO("start the test");
    
    Server tcp_server;
    tcp_server.start_multi(nullptr,12345);

    client_test();

	minico::sche_join();
	std::cout << "end" << std::endl;
	return 0;
}
