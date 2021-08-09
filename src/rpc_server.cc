
#include "../include/rpc_server.h"
#include <unordered_map>

/** 编解码器 是否需要 需要的话就加一层rpc信息头部的封装 8 bytes*/
struct RpcHeader
{
    uint16_t info;
    uint16_t magic;
    uint32_t len;   /** 标识rpc消息的长度*/
} 

static const uint16_t DefaultMagic = 0x7777;

/** 设置rpc头部信息到header中,rpc的消息体的长度为msg_len*/
void set_rpc_header(void* header,int msg_len)
{
    ((RpcHeader*) header)->magic = DefaultMagic;
    ((RpcHeader*) header)->len = hton32(msg_len);
}


/** 
 * 基础的心跳服务 默认
*/
class Ping : public Service
{
  public:
    Ping() = default;
    virtual ~Ping() = default;

    virtual const char* name() const override { return "ping";}

    virtual void process(const TinyJson& request,TinyJson& result) override
    {
        result["err"].Set(200);
        result["errmsg"].Set("pong");
    }
};

/**
 * @brief 服务器的可复用实际功能实现 基础功能添加了心跳服务
*/
class ServerImpl
{
  public:
    ServerImpl() : _conn_number(0) 
    {
        this->add_service(new Ping);
    }

    ~ServerImpl() = default;

    /** 向服务器注册一种新的服务*/
    void add_service(Service* s)
    {
        _services[s->name()] = std::shared_ptr<Service>(s);
    }

    /** 利用服务名查询是否存在该服务*/
    Service* find_service(const std::string& name)
    {
        auto it = _services.find(name);
        if(it != _services.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

    /** 实际接收客户端连接时的回调函数*/
    void on_connection(minico::Socket* conn);

    /**
     * @brief 复用基础的tcp服务器,先注册自定义的服务器实际的连接回调函数,再开启运行
    */
    void start(const char* ip,int port)
    {
        /** 注册自定义的连接回调函数*/
        _tcp_server.register_connection(&ServerImpl::on_connection,this);
        /** 开启服务器运行*/
        _tcp_server.start(ip,port);

    }

    /** 业务处理逻辑函数*/
    void process(const TinyJson& request,TinyJson& result);

  private:
    /** 基础的tcp服务器,这里先保存了一个新的服务器*/
    Server _tcp_server;

    /** 保存的客户端的连接数*/
    int _conn_number;

    /** 保存的服务的列表*/
    std::unordered_map<std::string,std::shared_ptr<Service>> _services;
};


/**
 * @brief 业务逻辑:找到相应的服务并执行对应的处理逻辑
*/
void ServerImpl::process(const TinyJson& request,TinyJson& result)
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
void ServerImpl::on_connection(minico::Socket* conn)
{
    /** 进行conn-fd的生命期管理*/
    std::unique_ptr<minico::Socket> connection(conn);

    RpcHeader rpc_header;

    TinyJson request;
    TinyJson result;

    std::vector<char> buf;

    int received_bytes = 0;
    int rpc_recv_message_len = 0;
    int rpc_send_message_len = 0;

    /** 
     * 收到了客户端发出的rpc请求,会做出如下处理 先不考虑错误处理
     * rpc请求会先收到一个头部信息,用于后续的主体信息流的截取 
     */
    while(true)
    {
        /** 接收规定大小的rpc的头部信息到header中*/
        received_bytes = conn->read(&rpc_header,sizeof(rpc_header));
        
        /** 拿到收到的rpc的信息的长度*/
        rpc_recv_message_len = ntoh32(rpc_header.len);

        /** 对缓冲区进行初步处理 调整大小用于接收rpc实际数据信息,并接收信息*/
        buf.resize(rpc_recv_message_len); 
        received_bytes = conn->read((void*)buf[0],rpc_recv_message_len);

        /** 将接收到的rpc请求从字节流转换为一个json对象*/
        std::string str_json_request(buf.begin(),buf.end());
        request.ReadJson(str_json_request);

        /** 打印出来*/
        LOG_INFO("json message is %s",request.WriteJson());

        /** rpc信息接收完毕,以下是处理逻辑*/
        this->process(request,result);

        /** json->string->vector<char>buf*/
        std::string str_json_result = result.WriteJson();
        rpc_send_message_len = str_json_result.length();
        buf.clear();
        buf.resize(sizeof(RpcHeader) + rpc_send_message_len);

        /** 在发送缓冲区中填入rpc头部信息*/
        set_rpc_header((void*)&buf[0],rpc_send_message_len);



    }
}



RpcServer::RpcServer()
{
    _p = new ServerImpl;
}

RpcServer::~RpcServer()
{
    delete (ServerImpl*)_p;
}

void RpcServer::add_service(Service* s)
{
    ((ServerImpl*)_p)->add_service(s);
}

void RpcServer::start(const char* ip,int port)
{
    ((ServerImpl*)_p)->start(ip,port);
}