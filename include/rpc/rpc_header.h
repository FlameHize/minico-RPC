#pragma once

#include <arpa/inet.h>
#include <stdint.h>

/**
 * rpc header
*/
/** 编解码器 是否需要 需要的话就加一层rpc信息头部的封装 8 bytes*/
struct RpcHeader
{
    uint16_t info;
    uint16_t magic;
    uint32_t len;   /** 标识rpc消息的长度*/
};

/** 设置rpc头部信息到header中,rpc的消息体的长度为msg_len*/
void set_rpc_header(void* header,int msg_len);