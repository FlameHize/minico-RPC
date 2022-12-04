#include "../../include/udp/udp_client.h"

void UdpClient::connect(const char* ip,int port)
{
    /** 调用client_socket的连接函数*/
    LOG_INFO("the udpclient connection to the server");
    serv_addr = new sockaddr_in();
    //memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr->sin_family = AF_INET;
    inet_pton(AF_INET,ip,&serv_addr->sin_addr);
    serv_addr->sin_port = htons(port);
}

ssize_t UdpClient::recvfrom(int sockfd, void* buf, int len, unsigned int flags,
						sockaddr* from, socklen_t* fromlen)
{
	// Modify: 当前协程恢复运行 那么就继续读
	return m_client_socket->recvfrom(sockfd, buf, len, flags, from, fromlen);
}

ssize_t UdpClient::sendto(int sockfd, const void* buf, int len, unsigned int flags,
						const struct sockaddr* to, int tolen)
{
	return m_client_socket->sendto(sockfd, buf, len, flags, to, tolen);
}









