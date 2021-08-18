#include "../../include/rpc/rpc_server.h"
#include "../../include/rpc/rpc_header.h"

void RpcServer::start(const char* ip,int port)
{
    std::function<void(minico::Socket*)> on_connection = 
        std::bind(&RpcServer::on_connection,this,std::placeholders::_1);
    
    /** register the connection callback*/
    m_rpc_server_stub->register_connection(on_connection);
    LOG_INFO("register the rpc-server-stub connection callback");

    m_rpc_server_stub->start(ip,port);
}

void RpcServer::start_multi(const char* ip,int port)
{
    std::function<void(minico::Socket*)> on_connection = 
        std::bind(&RpcServer::on_connection,this,std::placeholders::_1);
    
    /** register the connection callback*/
    m_rpc_server_stub->register_connection(on_connection);
    LOG_INFO("register the rpc-server-stub connection callback");

    m_rpc_server_stub->start_multi(ip,port);
}

void RpcServer::process(TinyJson& request,TinyJson& result)
{
    /** 解析客户端传入的服务key-value,获取对应的服务名称*/
    std::string service = request.Get<std::string>("service");
    LOG_INFO("the request service is %s",service.c_str());
    /** 说明用户配置了service的key-value*/
    if(!service.empty())
    {
        /** 通过服务的名称在服务的注册表中找到对应的服务类*/
        auto s = this->find_service(service);
        if(s)
        {
            LOG_INFO("find the %s -service",s->name());
            /** 如果找到对应的服务类,就调用该服务对象进行业务逻辑处理*/
            s->process(request,result);
        }
        else
        {
            result["err"].Set(404);
            result["errmsg"].Set("service not found");
        }
    }
    else
    {
        result["err"].Set(400);
        result["errmsg"].Set("request has not config service");
    }
    return;
}

void RpcServer::on_connection(minico::Socket* conn)
{
    /** 进行conn-fd的生命期管理*/
    std::unique_ptr<minico::Socket> connection(conn);

    /** add one client connection*/

    RpcHeader rpc_header;
    std::vector<char> buf;

    int rpc_recv_message_len = 0;
    /** 
     * 收到了客户端发出的rpc请求,会做出如下处理 先不考虑错误处理
     * rpc请求会先收到一个头部信息,用于后续的主体信息流的截取
     * 两次接收 一次发送 
     */
    while(true)
    {
        TinyJson request;
        TinyJson result;

        /** 接收规定大小的rpc的头部信息到header中*/
        int rpc_request_message_len = 
            connection->read(&rpc_header,sizeof(rpc_header));
        //LOG_INFO("the rpc-server-stub received rpc_header len is %d",
        //    rpc_request_message_len);

        /** for client send exit and process*/
        if(rpc_request_message_len == 0)
        {
            LOG_INFO("detect a client exit,rpc-server-stub should break the connection");
            break;
        }
        /** 拿到收到的rpc的信息的长度 网络序需要转换为主机字节序*/
        rpc_recv_message_len = ntohl(rpc_header.len);
        //LOG_INFO("the receive rpc message len is %d",rpc_recv_message_len);

        /** 对缓冲区进行初步处理 调整大小用于接收rpc实际数据信息,并接收信息*/
        buf.clear();
        buf.resize(rpc_recv_message_len); 
        connection->read((void*)&buf[0],rpc_recv_message_len);

        /** 将接收到的clent-rpc请求从字节流转换为一个json对象*/
        m_rpc_server_stub->encode(buf,request);

        /** 交给上层rpc业务服务器的业务处理逻辑*/
        process(request,result);

        /** 把将处理后得到的json转换成字节流*/
        m_rpc_server_stub->decode(buf,result);

        /** 一次性将数据全部发送出去*/
        connection->send((void*)&buf[0],buf.size());
    }
}

