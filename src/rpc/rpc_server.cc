#include "../../include/rpc/rpc_server.h"
#include "../../include/rpc/rpc_header.h"

void RpcServer::start(const char* ip,int port)
{
    /** 注册自定义的连接回调函数,这里需要注意占位符问题*/
    std::function<void(minico::Socket*)> on_connection = 
        std::bind(&RpcServer::on_connection,this,std::placeholders::_1);

    /** register the connection callback*/
    m_tcp_server->register_connection(on_connection);
    LOG_INFO("register the serverimpl connection");

    /** 开启tcp服务器运行*/
    m_tcp_server->start(ip,port); 
    LOG_INFO("rpcserver start the tcpserver run");
}

void RpcServer::process(TinyJson& request,TinyJson& result)
{
    /** 解析客户端传入的服务key-value,获取对应的服务名称*/
    std::string service = request.Get<std::string>("service");
    
    /** 说明用户配置了service的key-value*/
    if(!service.empty())
    {
        /** 通过服务的名称在服务的注册表中找到对应的服务类*/
        auto s = this->find_service(service);
        if(s)
        {
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

/**
 * @brief 实际需要实现的连接回调函数 需要注入到基础的tcp服务器中
*/
void RpcServer::on_connection(minico::Socket* conn)
{
    /** 进行conn-fd的生命期管理*/
    std::unique_ptr<minico::Socket> connection(conn);

    /** add one client connection*/
    ++m_conn_number;

    RpcHeader rpc_header;

    TinyJson request;
    TinyJson result;

    std::vector<char> buf;

    int rpc_recv_message_len = 0;
    int rpc_send_message_len = 0;

    /** 
     * 收到了客户端发出的rpc请求,会做出如下处理 先不考虑错误处理
     * rpc请求会先收到一个头部信息,用于后续的主体信息流的截取
     * 两次接收 一次发送 
     */
    while(true)
    {
        /** 接收规定大小的rpc的头部信息到header中*/
        int received_len = connection->read(&rpc_header,sizeof(rpc_header));
        LOG_INFO("the receive rpc_header len is %d",received_len);

        /** 拿到收到的rpc的信息的长度 网络序需要转换为主机字节序*/
        rpc_recv_message_len = ntohl(rpc_header.len);
        LOG_INFO("the receive rpc message len is %d",rpc_recv_message_len);

        /** 对缓冲区进行初步处理 调整大小用于接收rpc实际数据信息,并接收信息*/
        buf.clear();
        buf.resize(rpc_recv_message_len); 
        connection->read((void*)&buf[0],rpc_recv_message_len);

        /** 将接收到的rpc请求从字节流转换为一个json对象*/
        std::string str_json_request(buf.begin(),buf.end());
        LOG_INFO("the receive rpc meessage is %s",str_json_request.c_str());
        
        /** 编码形成一个json对象*/
        request.ReadJson(str_json_request);

        /** rpc信息接收完毕,以下是处理逻辑*/
        this->process(request,result);

        /** json->string->vector<char>buf*/
        std::string str_json_result = result.WriteJson();
        LOG_INFO("the process rpc message result is %s",str_json_result.c_str());
        rpc_send_message_len = str_json_result.length();
        LOG_INFO("the process rpc message result len is %d",rpc_send_message_len);

        buf.clear();
        buf.resize(sizeof(RpcHeader) + rpc_send_message_len);

        /** 在发送缓冲区中填入rpc头部信息*/
        set_rpc_header((void*)&buf[0],rpc_send_message_len);

        /** 采用vector::copy来进行数据的复制 这里需要进行测试*/
        std::vector<char>::iterator begin = buf.begin();
        for(unsigned int i = 0;i < sizeof(RpcHeader);++i)
        {
            ++begin;
        }
        /** 进行拷贝,理论上是无问题的*/
        std::copy(str_json_result.begin(),str_json_result.end(),begin);

        std::string rpc_server_send_result(buf.begin(),buf.end());
        std::cout << "the rpc server send result string is" 
            << rpc_server_send_result << std::endl;

        /** 一次性将数据全部发送出去*/
        connection->send((void*)&buf[0],buf.size());
    }
    /** client exit */
    --m_conn_number;
}