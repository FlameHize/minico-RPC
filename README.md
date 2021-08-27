Minico-RPC
=====
介绍
---
minico-rpc是一个纯C++11实现的轻量级RPC服务器框架，框架在底层实现了一个高性能的多线程/多协程调度器，并对socket进行了类"hook"处理，同时在上层提供支持TCP连接和json协议约定的RPC服务 <br>

特性
---
* 基于boost::ucontext函数集开发了协程对象，除此之外无需再引入任何第三方库 <br>
* 实现了线程/协程调度器，自带负载均衡，同时支持用户指定CPU核心数和特定CPU核心来运行任务 <br>
* 可根据实际需要动态的配置协程栈大小，同时配置了内存池提升多协程的调度速度 <br>
* 将复杂的异步处理流程隐藏在框架底层，通过类似Golang接口的`minico::co_go()`完成协程接口封装，上层用户可以使用业务同步开发的方式获得异步的高性能，避免了异步回调和回调分离引发的代码支离破碎 <br>
* 对socket进行了类hook处理，用户可以使用和系统socket相同的接口来开发自己的网络服务器，同时项目内也提供了非阻塞模式下协程TCP/RPC服务器/客户端实现(客户端必须在协程中运行) <br>
* 用户通过继承Service类并实现自己的服务接口，在RPC服务器中添加服务后，即可实现相应的服务 <br>


使用
---
项目采用CMake进行编译,在工程主目录下新建build目录，进入build目录 <br>
```
cmake ..
make  
```

编译完成后，TCP和RPC服务器可执行程序位于bin中

示例
---
TCP服务器和客户端 <br>
``` cpp
/** 客户端程序--运行在一个协程中*/
void tcp_client_worker(TcpClient& tcp_client)
{
    tcp_client.connect("127.0.0.1",12345);
    char buf[1024];
    tcp_client.send("ping",4);
    tcp_client.recv(buf,1024);
    LOG_INFO("client recv %s",buf);
}

int main()
{
    TcpServer tcp_server;
    tcp_server.start("127.0.0.1",12345);
    TcpClient tcp_client;
    minico::co_go([&tcp_client](){
		tcp_client_worker(tcp_client);
	});
    minico::sche_join();
    return 0;
}
```
RPC服务器和客户端 <br>
```cpp

/** 注册的一种RPC服务，包含多个接口*/
class HelloWorld : public Service
{
  public:
	typedef void (HelloWorld::*Func)(TinyJson& request,TinyJson& result);
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
	virtual void process(TinyJson& request,TinyJson& result) override
	{
		std::string method = request.Get<std::string>("method");
		LOG_INFO("the request method is %s",method.c_str());
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
	virtual void hello(TinyJson& request,TinyJson& result) = 0;
	virtual void world(TinyJson& request,TinyJson& result) = 0;
  private:
	std::unordered_map<std::string,Func> _methods;
	std::string _name;
};

class HelloWorldImpl : public HelloWorld
{
  public:
	HelloWorldImpl() = default;
	virtual ~HelloWorldImpl() = default;
	
	virtual void hello(TinyJson& request,TinyJson& result)
	{
		result["method"].Set("hello");
		result["err"].Set(200);
		result["errmsg"].Set("the loop problem is solved");
	}
	virtual void world(TinyJson& request,TinyJson& result)
	{
		result["method"].Set("world");
		result["err"].Set(200);
		result["errmsg"].Set("ok");
	}
};


/** RPC客户端处理数据的工作函数*/
void rpc_client_worker(RpcClient& rpc_client,int number)
{
    rpc_client.connect("127.0.0.1",12345);
    for(int i = 0; i < number; ++i)
    {
        LOG_INFO("-------the %d st client test-----------",i);
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
}

int main()
{
    RpcServer rpc_server;
    rpc_server.add_service(new HelloWorldImpl);		//添加一种RPC服务，包含多个方法接口
    rpc_server.start_multi(nullptr,12345);		//开启多线程运行
    RpcClient rpc_client;				
    int loop_time = 100;
	minico::co_go([&rpc_client,&loop_time](){
		rpc_client_worker(rpc_client,loop_time);
	});
    minico::sche_join();
    return 0;
}
```

目前的缺点
---
* co_go()仅支持绑定特定对象`std::function<void()>`，开发时需要和std::bind联合才能绑定多个参数，后续考虑加入变参模板，在协程和函数中间封装一层闭包任务对象 <br>
* RPC-stub层采用json协议对传入函数参数有一定的限制，且开发服务对象冗余过多，后续考虑加入脚本生成服务模板 <br>





