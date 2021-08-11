#include <iostream>
#include <string>
#include <unordered_map>
#include <sys/sysinfo.h>

#include "../include/processor.h"
#include "../include/minico_api.h"
#include "../include/socket.h"
#include "../include/mutex.h"

#include "../include/logger.h"
#include "../include/tcp_server.h"
#include "../include/rpc_server.h"

using namespace minico;

/** 注册的一种Service*/
class HelloWorld : public Service
{
  public:
	typedef void (*Func)(const TinyJson& request,TinyJson& result);

	HelloWorld() : _name("HelloWorld")
	{
		_methods["hello"] = &HelloWorld::hello;
		_methods["world"] = &HelloWorld::world;
	}
	virtual const char* name() const
	{
		return _name.c_str();
	}
	virtual ~HelloWorld() {}

	/** 实际的服务类的处理函数*/
	virtual void process(const TinyJson& request,TinyJson& result)
	{
		std::string method = request.Get<std::string>("method");
		if(method.empty())
		{
			result["err"].Set(400);
			result["errmsg"].Set("request has no method");
			return;
		}
		auto it = _methods.find(method);
		if(it == _methods.end())
		{
			result["err"].Set(404);
			result["errmsg"].Set("method not found");
			return;
		}
		/** 找到服务的对应接口 那么执行即可*/
		(this->*(it->second))(request,result);
	}
	/** 需要用户重载的实际逻辑部分*/
	virtual void hello(const TinyJson& request,TinyJson& result) = 0;
	virtual void world(const TinyJson& request,TinyJson& result) = 0;
  private:
	std::unordered_map<std::string,Func> _methods;
	std::string _name;
};

class HelloWorldImpl : public HelloWorld
{
  public:
	HelloWorldImpl() = default;
	virtual ~HelloWorldImpl() = default;
	
	virtual void hello(const TinyJson& request,TinyJson& result)
	{
		result["method"].Set("hello");
		result["err"].Set(200);
		result["errmsg"].Set("ok");
	}
	virtual void world(const TinyJson& request,TinyJson& result)
	{
		result["method"].Set("world");
		result["err"].Set(200);
		result["errmsg"].Set("ok");
	}
};

void client_func()
{
	RpcClient client("127.0.0.1",12345);
	TinyJson request;
	TinyJson result;
	request["service"].Set("HelloWorld");
	request["method"].Set("method","hello");
	client.call(request,result);
	return;
}

int main()
{

    LOG_INFO("start the test");
    
    RpcServer s;
	s.start(nullptr,12345);

	minico::co_go(client_func);
	minico::sche_join();
	std::cout << "end" << std::endl;
	return 0;
}
