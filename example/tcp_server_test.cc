#include <iostream>
#include <string>
#include <sys/sysinfo.h>

#include "../include/logger.h"
#include "../include/tcp/tcp_server.h"


int main()
{
    LOG_INFO("---------------");
    LOG_INFO("TEST TCP SERVER");
    LOG_INFO("---------------");

    // Default: ping-pong
    TcpServer tcp_server;
    tcp_server.start("127.0.0.1",8888);
    minico::sche_join();
    return 0;
}