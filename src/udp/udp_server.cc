#include "../../include/udp/udp_server.h"

// UdpServer::conn_callback Udp_default_connection(
//     [](minico::Socket* co_socket)
//     {
//         sockaddr_in client_addr;
//         socklen_t client_addr_len = sizeof(client_addr);
//         std::vector<char> buf;
//         buf.resize(2048);
//         while(true)
//         {
//             LOG_INFO("--------start one read-write process loop------------");
//             /** readnum是实际上读到的数据*/
//             auto readNum = co_socket->recvfrom(co_socket->fd(), (void*)buf[0], buf.size(),
//                                 0, (struct sockaddr*)&client_addr, &client_addr_len);
//             if(readNum <= 0)
//             {
//                 /** read = 0 exit*/
//                 break;
//             }
//             co_socket->sendto(co_socket->fd(), (void*)buf[0], readNum, 0, 
//                             (struct sockaddr*)&client_addr, client_addr_len);
//             LOG_INFO("--------finish one read-write process loop------------");
//         }
//     }
// );

/** 
 *服务器开启运行 需要拿到配置 然后开启连接
 */
void UdpServer::start(const char* ip, int port)
{
    /** 创建一个socket,进行服务器的参数的配置*/
    _listen_fd = new minico::Socket("UDP");
    if(_listen_fd->isUseful())
    {
        LOG_INFO("the udp server fd %d is useful",_listen_fd);
        _listen_fd->setTcpNoDelay(true);
        _listen_fd->setReuseAddr(true);
        _listen_fd->setReusePort(true);
        // udp服务器的套接字直接bind到指定ip和port上 无需监听
        if(_listen_fd->bind(ip,port) < 0)
        {
            LOG_ERROR("udp server start error");
            return;
        }
        
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
    auto loop = std::bind(&UdpServer::server_loop,this);
    
    /** 需要start方法非阻塞,因此需要再开启一个协程来运行*/
    minico::co_go(loop);
    return;
}


/** 
 *@brief 服务器工作函数
 */
void UdpServer::server_loop()
{
    LOG_INFO("-----------------------------");
    LOG_INFO("start run the udp server loop");
    LOG_INFO("-----------------------------");
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    std::vector<char> buf;
    buf.resize(2048);
    while(true)
    {
        LOG_INFO("--------start one read-write process loop------------");
        /** readnum是实际上读到的数据*/
        auto readNum = _listen_fd->recvfrom(_listen_fd->fd(), (void*)buf[0], buf.size(),
                            0, (struct sockaddr*)&client_addr, &client_addr_len);
        if(readNum <= 0)
        {
            /** read = 0 exit*/
            break;
        }
        _listen_fd->sendto(_listen_fd->fd(), (void*)buf[0], readNum, 0, 
                        (struct sockaddr*)&client_addr, client_addr_len);
        LOG_INFO("--------finish one read-write process loop------------");
    }
    LOG_INFO("udpserver exit");
    return;
}



