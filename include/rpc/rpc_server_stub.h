#pragma once

#include "../../include/rpc/rpc_header.h"
#include "../../include/tcp/tcp_server.h"
#include "../../include/json.h"

/** 前向声明*/
class RpcServer;

class RpcServerStub
{
  public:
    DISALLOW_COPY_MOVE_AND_ASSIGN(RpcServerStub);

    RpcServerStub() : m_tcp_server(new TcpServer())
    {
        LOG_INFO("rpc server constructor a tcp server");
    }

    ~RpcServerStub()
    {
        delete m_tcp_server;
        m_tcp_server = nullptr;
        LOG_INFO("rpc_client destructor a tcp server");
    }

    void start(const char* ip,int port);
    void start_multi(const char* ip,int port);

    void register_connection(std::function<void(minico::Socket*)>& conn);

    /**
     * encode + decode
    */
    // 将接收到的clent-rpc请求从字节流转换为一个json对象
    void encode(std::vector<char>& buf,TinyJson& request);

    // 把将处理后得到的json转换成字节流
    void decode(std::vector<char>& buf,TinyJson& result);

    void process(TinyJson& request,TinyJson& result);

  private:
    /** connection callback*/
    void on_connection(minico::Socket* conn);
    
    TcpServer* m_tcp_server;


};