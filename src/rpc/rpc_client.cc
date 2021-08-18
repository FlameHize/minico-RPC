#include "../../include/rpc/rpc_client.h"
#include "../../include/rpc/rpc_header.h"


void RpcClient::call(TinyJson& request,TinyJson& result)
{
    /** 将json请求转换成字节流（解码）并发送*/
    m_rpc_client_stub->decode(request);

    /** 将收到的字节流转换（编码）成json对象并返回*/
    m_rpc_client_stub->encode(result);
}


void RpcClient::ping()
{
    LOG_INFO("enter the client ping");
    TinyJson request;
    TinyJson result;
    request["service"].Set("ping");
    this->call(request,result);
    // std::string ping_result = result.WriteJson();
    // LOG_INFO("the rpc client process rpc message is %s",
    //     ping_result.c_str());
    // std::cout << "the rpc client process rpc message is " 
    //     << ping_result << std::endl;
    int errcode = result.Get<int>("err");
    std::string errmsg = result.Get<std::string>("errmsg");
    LOG_INFO("-----------------------------");
    LOG_INFO("the ping errcode is %d",errcode);
    LOG_INFO("the ping errmsg is %s",errmsg.c_str());
    LOG_INFO("------------------------------");
    LOG_INFO("leave the clent ping");
}