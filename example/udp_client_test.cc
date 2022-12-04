#include <iostream>
#include <string>
#include <sys/sysinfo.h>

#include "../include/logger.h"
#include "../include/udp/udp_client.h"


void udp_client_worker(UdpClient& udp_client,int& loop_time)
{
    udp_client.connect("127.0.0.1",8888);
    socklen_t len = sizeof(*udp_client.serv_addr);
    std::vector<char> buf;
    buf.resize(2048);

    //LOG_INFO("client send ping");
    for(int i = 0; i < loop_time; ++i)
    {
        udp_client.sendto(udp_client.socket(),"ping",4,0,
                                (struct sockaddr*)udp_client.serv_addr,len);
        udp_client.recvfrom(udp_client.socket(),(void*)buf[0],buf.size(),0,
                                (struct sockaddr*)udp_client.serv_addr,&len);
        LOG_INFO("client %dth recv %s",i,buf);
    }
}

int main()
{
    LOG_INFO("---------------");
    LOG_INFO("TEST UDP CLIENT");
    LOG_INFO("---------------");

    // Default: ping-pong
    UdpClient udp_client;
    int loop_time  = 10;
    minico::co_go([&udp_client,&loop_time](){
		udp_client_worker(udp_client,loop_time);
	});
    minico::sche_join();
    return 0;
}