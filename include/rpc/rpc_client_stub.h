#pragma once

#include "../../include/rpc/rpc_header.h"
#include "../../include/tcp/tcp_client.h"
#include "../../include/json.h"

/**
 * RPC客户端存根，负责网络消息的发送与编解码
*/
class RpcClientStub
{
  public:
    DISALLOW_COPY_MOVE_AND_ASSIGN(RpcClientStub);

    RpcClientStub() : m_tcp_client(new TcpClient())
    {
        LOG_INFO("rpc client constructor a tcp client");
    }
    
    ~RpcClientStub()
    {
        delete m_tcp_client;
        m_tcp_client = nullptr;
        LOG_INFO("rpc_client destructor a tcp client");
    }
    
    void connect(const char* ip,int port)
    {
        return m_tcp_client->connect(ip,port);
    }

    int close()
    {
        return m_tcp_client->disconnect();
    }

    /**
     * encode + decode
    */
    //接受服务端传回来的消息 
    //将接收到的rpc请求从buf字节流转换为一个json对象并从形参返回
    void encode(TinyJson& result);

    //把写入的json转换为string格式 拼接头部和消息主体 发送信息
    void decode(TinyJson& request);

  private:
    TcpClient* m_tcp_client;
    std::vector<char> buf;
};