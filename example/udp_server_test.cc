#include <iostream>
#include <string>
#include <sys/sysinfo.h>

#include "../include/logger.h"
#include "../include/udp/udp_server.h"


int main()
{
    LOG_INFO("---------------");
    LOG_INFO("TEST UDP SERVER");
    LOG_INFO("---------------");

    // Default: ping-pong
    UdpServer udp_server;
    udp_server.start("127.0.0.1",8888);
    minico::sche_join();
    return 0;
}