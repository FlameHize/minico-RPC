#include "../../include/rpc/rpc_client.h"
#include "../../include/rpc/rpc_header.h"


void RpcClient::call(TinyJson& request,TinyJson& result)
{
    /** 先把写入的json转换为string格式*/
    std::string str_json_request = request.WriteJson();
    LOG_INFO("the rpc client call request is %s",
    str_json_request.c_str());
    LOG_INFO("thr rpc client call request len is %d",str_json_request.length());

    int rpc_send_message_len = str_json_request.length();

    /** 拼接头部和消息主体 发送信息*/
    buf.clear();
    buf.resize(sizeof(RpcHeader) + rpc_send_message_len);
    set_rpc_header((void*)&buf[0],rpc_send_message_len);
    std::vector<char>::iterator begin = buf.begin();
    for(unsigned int i = 0; i < sizeof(RpcHeader);++i)
    {
        ++begin;
    }
    std::copy(str_json_request.begin(),str_json_request.end(),begin);
    m_tcp_client->send((void*)&buf[0],buf.size());

    //for test
    std::string rpc_client_send_string(buf.begin(),buf.end());
    const char* rpc_client_send = rpc_client_send_string.c_str();
    LOG_INFO("the rpc client send request string is %s",rpc_client_send);
    
    std::cout << "the rpc client send request string is " 
        << rpc_client_send_string << std::endl;

    /** 等待 接受服务端传回来的消息*/
    RpcHeader rpc_header;
    int rpc_recv_message_len = 0;
    m_tcp_client->recv(&rpc_header,sizeof(rpc_header));

    /** 拿到收到的rpc的信息的长度 字节序需要转换为网络序*/
    rpc_recv_message_len = ntohl(rpc_header.len);
    LOG_INFO("the rpc client receive process rpc message len is %d",
        rpc_recv_message_len);
        
    buf.clear();
    buf.resize(rpc_recv_message_len);
    m_tcp_client->recv((void*)&buf[0],rpc_recv_message_len);

    /** 将接收到的rpc请求从字节流转换为一个json对象*/
    std::string str_json_result(buf.begin(),buf.end());
    LOG_INFO("the rpc client process rpc message is %s",
        str_json_result.c_str());

    result.ReadJson(str_json_result);
    return;
}


void RpcClient::ping()
{
    TinyJson request;
    TinyJson result;
    request["service"].Set("ping");
    this->call(request,result);
    std::string ping_result = result.WriteJson();
    LOG_INFO("the rpc client process rpc message is %s",
        ping_result.c_str());
    std::cout << "the rpc client process rpc message is " 
        << ping_result << std::endl;
    int errcode = result.Get<int>("err");
    std::string errmsg = result.Get<std::string>("errmsg");
    LOG_INFO("the ping errcode is %d",errcode);
    LOG_INFO("the ping errmsg is %s",errmsg.c_str());
}