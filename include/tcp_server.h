#pragma once

#include <functional>
#include <memory>
#include <sys/sysinfo.h>

#include "../include/logger.h"
#include "../include/socket.h"
#include "../include/minico_api.h"

/**
 * @brief 基于协程的tcp服务器 一个连接对应一个协程
 * 是一个基类,可以被支持不同业务的服务器复用
 */
class Server
{
public:
    /** 用户注册的服务器的连接回调函数 这里需要注意connection的生命期*/
    typedef std::function<void(minico::Socket*)> conn_callback;
    
    Server() : _listen_fd(nullptr),_multi_listen_fd(nullptr),
        server_ip(nullptr),server_port(-1) {
        LOG_INFO("constructor the tcp server finish");
    }
    
    virtual ~Server()
    {
        delete _listen_fd;
        _listen_fd = nullptr;
        delete[] _multi_listen_fd;
        _multi_listen_fd = nullptr;
        delete server_ip;
        server_ip = nullptr;
    }
    
    /** 单线程模式下开启服务器的运行*/
    virtual void start(const char* ip,int port);
    
    /** 多线程模式下开始服务器的运行*/
    virtual void start_multi(const char* ip,int port);
    
    /** 注册连接函数的接口 这里选择使用一层转发*/
    inline void register_connection(const conn_callback& func)
    {
        this->register_connection(conn_callback(func));
    }
    inline void register_connection(conn_callback&& func)
    {
        _on_server_connection.reset(new conn_callback(std::move(func)));
    }
    
private:
    /** 服务器的loop工作函数*/
    void server_loop();
    
    /** 服务器多核loop函数*/
    void multi_server_loop(int thread_number);
    
    /** 保存客户端连接时触发的回调函数 这里不要保存tcp连接*/
    std::shared_ptr<conn_callback> _on_server_connection;
    
    /** 用于监听的socket*/
    minico::Socket* _listen_fd;
    
    /** 多线程下监听的socket队列*/
    minico::Socket* _multi_listen_fd;
    
    /** 服务器的初始参数*/
    const char* server_ip;
    int server_port;
    
};

/**
 * @brief: 客户端 一个客户端只用来保存一个连接 客户端必须在一个协程中运行
 */
class Client
{
public:
    /** 进行socket的一系列初始化的工作*/
    Client(const char* ip,int port) : server_ip(ip),server_port(port)
    {
        /** 拿到一个socket*/
        client_socket = new minico::Socket();
    }
    Client(const Client& c) : server_ip(c.server_ip),server_port(c.server_port),client_socket(new minico::Socket()){

    }
    void operator=(const Client& c) = delete;
    virtual ~Client();
    
    void connect();
    void disconnect();
    int recv(void* buf,size_t count);
    int send(const void* buf,size_t count);
    
    inline int socket() const { return client_socket->fd();}
    
private:    
    const char* server_ip;
    int server_port;
    minico::Socket*  client_socket;
};


















