#include <iostream>
#include <string>
#include <sys/sysinfo.h>

#include "../include/logger.h"
#include "../include/tcp/tcp_client.h"


void tcp_client_worker(TcpClient& tcp_client,int& loop_time)
{
    tcp_client.connect("127.0.0.1",8888);
    char buf[2048];

    //LOG_INFO("client send ping");
    for(int i = 0; i < loop_time; ++i)
    {
        tcp_client.send("ping",4);
        tcp_client.recv(buf,2048);
        LOG_INFO("client %dth recv %s",i,buf);
    }
}

int main()
{
    LOG_INFO("---------------");
    LOG_INFO("TEST TCP CLIENT");
    LOG_INFO("---------------");

    // Default: ping-pong
    TcpClient tcp_client;
    int loop_time  = 10;
    minico::co_go([&tcp_client,&loop_time](){
		tcp_client_worker(tcp_client,loop_time);
	});
    minico::sche_join();
    return 0;
}