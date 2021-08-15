#pragma once
#include "../../include/json.h"

/**
 * RPC服务器采用JSON格式来传输数据,而不是protobuf thrift等二进制协议
 * RpcServer接收到RPC请求时，根据request中的"service"字段找到对应的service
 * 然后调用该service的process()方法处理该请求
*/

/** Service是一个接口,表示一种服务,一个RpcServer可以包含多个Service*/
class Service
{
  public:
    Service() = default;
    virtual ~Service() = default;

    virtual const char* name() const  = 0;

    /** 实际进行服务数据处理的功能函数 可以更改为不同的策略*/
    virtual void process(TinyJson& request,TinyJson& result) = 0;
};

/** 
 * 基础的心跳服务 默认
*/
class Ping : public Service
{
  public:
    Ping() = default;
    virtual ~Ping() = default;

    virtual const char* name() const override { return "ping";}

    virtual void process(TinyJson& request,TinyJson& result) override
    {
        result["err"].Set(200);
        result["errmsg"].Set("pong");
    }
};