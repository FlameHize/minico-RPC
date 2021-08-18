#include "../../include/rpc/rpc_server_stub.h"

void RpcServerStub::start(const char* ip,int port)
{
    /** 开启tcp服务器运行*/
    m_tcp_server->start(ip,port); 
    LOG_INFO("rpc-server-stub start run the tcp-server loop");
}

void RpcServerStub::start_multi(const char* ip,int port)
{
    m_tcp_server->start_multi(ip,port);
    LOG_INFO("rpc-server-stub start run the tcp-server multi loop");
}

void RpcServerStub::register_connection(std::function<void(minico::Socket*)>& conn)
{
    m_tcp_server->register_connection(conn);
}

/**
 * 将socket接收到到信息保存到buf中并转换成json对象
*/
void RpcServerStub::encode(std::vector<char>& buf,TinyJson& request)
{
    /** 将接收到的rpc请求从字节流转换为一个json对象*/
    std::string str_json_request(buf.begin(),buf.end());
    LOG_INFO("the rpc-server-stub received message is %s",
        str_json_request.c_str());

    /** 编码形成一个json对象*/
    request.ReadJson(str_json_request);
}

/**
 * 将处理得到的result解码并保存到缓冲区buf
*/
void RpcServerStub::decode(std::vector<char>& buf,TinyJson& result)
{
    /** json decode*/
    std::string str_json_result = result.WriteJson();
    LOG_INFO("the processed rpc message result is %s",
        str_json_result.c_str());
    int rpc_result_message_len = str_json_result.length();

    /** send byte stream*/
    buf.clear();
    buf.resize(sizeof(RpcHeader) + rpc_result_message_len);

    /** 在发送缓冲区中填入rpc头部信息*/
    set_rpc_header((void*)&buf[0],rpc_result_message_len);

    std::vector<char>::iterator begin = buf.begin();
    for(unsigned int i = 0;i < sizeof(RpcHeader);++i)
    {
        ++begin;
    }
    /** 进行拷贝,理论上是无问题的*/
    std::copy(str_json_result.begin(),str_json_result.end(),begin);
    return;
}

