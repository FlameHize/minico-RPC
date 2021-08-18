#include "../../include/rpc/rpc_client_stub.h"

void RpcClientStub::decode(TinyJson& request)
{
    /** 先把写入的json转换为string格式*/
    std::string str_json_request = request.WriteJson();

    /** 解析得到消息主体的长度*/
    int rpc_request_message_len = str_json_request.length();

    /** 打包rpc标识头部和消息主体，发送网络消息*/
    buf.clear();
    buf.resize(sizeof(RpcHeader) + rpc_request_message_len);
    set_rpc_header((void*)&buf[0],rpc_request_message_len);
    std::vector<char>::iterator begin = buf.begin();
    for(unsigned int i = 0; i < sizeof(RpcHeader); ++i){
        ++begin;
    }
    std::copy(str_json_request.begin(),str_json_request.end(),begin);
    m_tcp_client->send((void*)&buf[0],buf.size());

    //for test
    //std::string rpc_client_send_string(buf.begin(),buf.end());
    //const char* rpc_client_send = rpc_client_send_string.c_str();
    //LOG_INFO("the rpc client send request string is %s",rpc_client_send);
    
    //std::cout << "the rpc client send request string is " 
    //    << rpc_client_send_string << std::endl;
    return;
}

void RpcClientStub::encode(TinyJson& result)
{
    /** 等待 接受服务端传回来的消息*/
    RpcHeader rpc_header;
    int rpc_result_message_len = 0;
    m_tcp_client->recv(&rpc_header,sizeof(rpc_header));

    /** 解析得到消息主体的长度*/
    rpc_result_message_len = ntohl(rpc_header.len);

    /** 根据指示消息长度接收rpc消息主体*/
    buf.clear();
    buf.resize(rpc_result_message_len);
    m_tcp_client->recv((void*)&buf[0],rpc_result_message_len);

    /** 将接收到的rpc请求从字节流转换为一个json对象*/
    std::string str_json_result(buf.begin(),buf.end());
    result.ReadJson(str_json_result);

    //for test
    //LOG_INFO("the rpc client process rpc message is %s",
    //    str_json_result.c_str());
    return;

}