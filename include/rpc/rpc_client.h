#pragma once

#include "../../include/tcp/tcp_client.h"
#include "service.h"
#include "../../include/json.h"

/**
 * rpc客户端，其功能必须要在一个协程中运行
*/
class RpcClient
{
  public:
    DISALLOW_COPY_MOVE_AND_ASSIGN(RpcClient);

    RpcClient() : m_tcp_client(new TcpClient())
    {
      LOG_INFO("rpc_client constructor a tcp_client");
    }

    ~RpcClient()
    {
      LOG_INFO("rpc_client destructor a tcp_client");
      delete m_tcp_client;
      m_tcp_client = nullptr;
    }

    void connect(const char* ip,int port)
    {
      return m_tcp_client->connect(ip,port);
    }
    /**
     * @brief 进行一次rpc请求，从result形参中直接得到结果
    */
    void call(TinyJson& request,TinyJson& result);

    /** 客户端心跳检测*/
    void ping();

    /** close the rpcclient function*/
    int close()
    {
      return m_tcp_client->disconnect();
    }
  
  private:
    TcpClient* m_tcp_client;
    std::vector<char> buf;
};