#pragma once

#include "../../include/rpc/rpc_client_stub.h"
#include "../../include/json.h"

/**
 * rpc客户端，其功能必须要在一个协程中运行
*/
class RpcClient
{
  public:
    DISALLOW_COPY_MOVE_AND_ASSIGN(RpcClient);

    RpcClient() : m_rpc_client_stub(new RpcClientStub())
    {
      LOG_INFO("rpc_client constructor a rpc-client-stub");
    }
  
    ~RpcClient()
    {
      delete m_rpc_client_stub;
      m_rpc_client_stub = nullptr;
      LOG_INFO("rpc_client destructor a rpc-client-stub");
    }

    void connect(const char* ip,int port)
    {
      return m_rpc_client_stub->connect(ip,port);
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
      return m_rpc_client_stub->close();
    }
  
  private:
    RpcClientStub* m_rpc_client_stub;

};