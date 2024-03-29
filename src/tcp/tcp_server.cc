#include "../../include/tcp/tcp_server.h"

/** 默认的server自定义函数*/
TcpServer::conn_callback default_connection(
    [](minico::Socket* co_socket)
    {
        LOG_INFO("add one client connection");
        /** 对这个改造的socket进行生命其管理*/
        std::unique_ptr<minico::Socket> connect_socket(co_socket);
        
        /** 设置数据缓冲区*/
        std::vector<char> buf;
        buf.resize(2048);
        while(true)
        {
            LOG_INFO("--------start one read-write process loop------------");
            /** readnum是实际上读到的数据*/
            auto readNum = connect_socket->read((void*)&(buf[0]), buf.size());	//这里也会发生协程切换(切回去是运行loop)
            //std::string ok = "HTTP/1.0 200 OK\r\nServer: minico/0.1.0\r\nContent-Type: text/html\r\n\r\n";
            if(readNum <= 0)
            {
                /** read = 0 exit*/
                break;
            }
            //connect_socket->send(ok.c_str(), ok.size());
            connect_socket->send((void*)&(buf[0]), readNum);
            /** 
             * 现在的情况是，rpc客户端对应的这个链接循环，做完所有的任务就挂掉了，需要退出
             * 因此下面的退出处理是必须的
             * */
            //if(readNum < (int)buf.size())
            //{
            //    break;
            //}
            LOG_INFO("--------finish one read-write process loop------------");
        }
    }
);

/** 
 *服务器开启运行 需要拿到配置 然后开启连接
 */
void TcpServer::start(const char* ip,int port)
{
    /** 如果用户没有注册自定义连接函数,那么就使用默认的*/
    if(_on_server_connection == nullptr)
    {
        LOG_INFO("user has not register the connection func,so the tcp_connection func is the default");
        register_connection(default_connection);
    }
    /** 创建一个socket,进行服务器的参数的配置*/
    _listen_fd = new minico::Socket();
    if(_listen_fd->isUseful())
    {
        LOG_INFO("the server listen fd %d is useful",_listen_fd);
        _listen_fd->setTcpNoDelay(true);
        _listen_fd->setReuseAddr(true);
        _listen_fd->setReusePort(true);
        if(_listen_fd->bind(ip,port) < 0)
        {
            LOG_ERROR("server start error");
            return;
        }
        /** 开启监听*/
        _listen_fd->listen();
        
        /** 保存服务器的ip和端口号*/
        if(ip != nullptr){
            server_ip = ip;
        }
        else{
            server_ip = "any address";
        }
        server_port = port;
        LOG_INFO("server ip is %s",server_ip);
        LOG_INFO("server port is %d",server_port);
    }
    /** 开始运行server loop*/
    auto loop = std::bind(&TcpServer::server_loop,this);
    
    /** 需要start方法非阻塞,因此需要再开启一个协程来运行*/
    minico::co_go(loop);
    return;
}

/** 多核运行的工作函数*/
void TcpServer::start_multi(const char* ip,int port)
{
    auto tCnt = ::get_nprocs_conf();
    /** 如果用户没有注册自定义连接函数,那么就使用默认的*/
    if(_on_server_connection == nullptr)
    {
        LOG_INFO("user has not register the connection func,so the tcp_connection func is the default");
        register_connection(default_connection);
    }
    _multi_listen_fd = new minico::Socket[tCnt];
    /** 每个线程对应一个连接*/
    for(int i = 0; i < tCnt; ++i)
    {
        if(_multi_listen_fd[i].isUseful())
        {
            LOG_INFO("the tcpserver listen fd is useful");
            _multi_listen_fd[i].setTcpNoDelay(true);
            _multi_listen_fd[i].setReuseAddr(true);
            _multi_listen_fd[i].setReusePort(true);
            if(_multi_listen_fd[i].bind(ip,port) < 0)
            {
                LOG_ERROR("server start error");
                return;
            }
            /** 开启监听*/
            _multi_listen_fd[i].listen();
        }
        /** 开始运行server loop*/
        auto loop = std::bind(&TcpServer::multi_server_loop,this,i);
    
        /** 开启对应cpu线程数的协程 并分配到每一个核上*/
        minico::co_go(loop,minico::parameter::coroutineStackSize,i);
    }
    return;
}

/** 
 *@brief 服务器工作函数
 */
void TcpServer::server_loop()
{
    LOG_INFO("-------------------------");
    LOG_INFO("start run the server loop");
    LOG_INFO("-------------------------");
    while(true)
    {
        /** conn即可以用来进行fd通信*/
        LOG_INFO("block in server_loop,has no new client accept");
        minico::Socket* conn = new minico::Socket(_listen_fd->accept());
        LOG_INFO("unblock,the server add a new tcpclient connection,the connect fd is %d",conn->fd());
        conn->setTcpNoDelay(true);
        /** 
         *运行绑定的用户工作函数
         *为了防止内存泄漏,这里需要对conn进行管理 也就是用户自身进行管理
         */
        auto user_connection = std::bind(*_on_server_connection,conn);
        minico::co_go(user_connection);
    }
    LOG_INFO("tcpserver exit");
    return;
}

/** 
 *@brief 服务器多核工作函数
 */
void TcpServer::multi_server_loop(int thread_number)
{
    LOG_INFO("start run the server loop");
    while(true)
    {
        /** conn即可以用来进行fd通信*/
        minico::Socket* conn = new minico::Socket(_multi_listen_fd[thread_number].accept());
        LOG_INFO("add one client socket");
        conn->setTcpNoDelay(true);
        /** 
         *运行绑定的用户工作函数
         *为了防止内存泄漏,这里需要对conn进行管理 也就是用户自身进行管理
         */
        auto user_connection = std::bind(*_on_server_connection,conn);
        minico::co_go(user_connection);
    }
    LOG_INFO("server exit");
    return;
}


