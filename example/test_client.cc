#include <iostream>
#include <string>
#include <sys/sysinfo.h>

#include "../include/logger.h"
#include "../include/rpc/rpc_client.h"

void tcp_client_worker(TcpClient& tcp_client)
{
    tcp_client.connect("127.0.0.1",12345);
    char buf[1024];

    LOG_INFO("client send ping");
    tcp_client.send("ping",4);
    tcp_client.recv(buf,1024);
    LOG_INFO("client recv %s",buf);
    /** 问题初步分析是由于rpc客户端销毁造成一直发送0造成的*/
}

void rpc_client_worker(RpcClient& rpc_client)
{
    rpc_client.connect("127.0.0.1",12345);
    rpc_client.ping();
    TinyJson request;
    TinyJson result;
    request["service"].Set<std::string>("HelloWorld");
    request["method"].Set<std::string>("world");
    rpc_client.call(request,result);
    int errcode = result.Get<int>("err");
    std::string errmsg = result.Get<std::string>("errmsg");
    LOG_INFO("--------------------------------");
    LOG_INFO("the result errcode is %d",errcode);
    LOG_INFO("the result errmsg is %s",errmsg.c_str());
    LOG_INFO("--------------------------------");
}

int main()
{
    LOG_INFO("test: add one rpc client");
    //TcpClient tcp_client_test;
    RpcClient rpc_client_test;
    //minico::co_go([&tcp_client_test](){
	//	tcp_client_worker(tcp_client_test);
	//});
	minico::co_go([&rpc_client_test](){
		rpc_client_worker(rpc_client_test);
	});
    minico::sche_join();
    return 0;
}