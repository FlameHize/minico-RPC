#pragma once

#include <functional>
#include <memory>
#include <sys/sysinfo.h>

#include "../../include/logger.h"
#include "../../include/socket.h"
#include "../../include/minico_api.h"

/**
 * @brief: 客户端 一个客户端只用来保存一个连接 客户端必须在一个协程中运行
 */
class UdpClient
{
public:
    /** 进行socket的一系列初始化的工作*/
    UdpClient() : m_client_socket(new minico::Socket("UDP")),serv_addr(nullptr)
    {
        LOG_INFO("udpclient constructor a connection socket");
    }

    DISALLOW_COPY_MOVE_AND_ASSIGN(UdpClient);

    virtual ~UdpClient()
    {
        delete serv_addr;
        serv_addr = nullptr;
        delete m_client_socket;
        m_client_socket = nullptr;
        LOG_INFO("udpclient destructor itself and the connection socket");
    }
    
    void connect(const char* ip,int port);

    ssize_t recvfrom(int sockfd, void* buf, int len, unsigned int flags,
						sockaddr* from, socklen_t* fromlen);
						
    ssize_t sendto(int sockfd, const void* buf, int len, unsigned int flags,
						const struct sockaddr* to, int tolen);
    
    inline int socket() const { return m_client_socket->fd();}

public:    
    minico::Socket* m_client_socket;
    sockaddr_in* serv_addr;
};


















