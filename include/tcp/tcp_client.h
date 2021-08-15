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
class TcpClient
{
public:
    /** 进行socket的一系列初始化的工作*/
    TcpClient() : m_client_socket(new minico::Socket())
    {
        LOG_INFO("tcpclient constructor a connection socket");
    }

    DISALLOW_COPY_MOVE_AND_ASSIGN(TcpClient);

    virtual ~TcpClient()
    {
        delete m_client_socket;
        m_client_socket = nullptr;
        LOG_INFO("tcpclient destructor itself and the connection socket");
    }
    
    void connect(const char* ip,int port);

    /** return 0 is success -1 is error*/
    int disconnect();
    size_t recv(void* buf,size_t count);
    size_t send(const void* buf,size_t count);
    
    inline int socket() const { return m_client_socket->fd();}
    
private:    
    minico::Socket* m_client_socket;
};


















