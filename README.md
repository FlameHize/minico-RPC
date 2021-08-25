介绍:minico-rpc是一个纯C++11实现的轻量级RPC服务器框架，框架底层实现了一个高性能的多线程下的协程网络库，对socket进行了类"hook"处理，同时在上层提供支持TCP连接和json协议约定的RPC服务
特性：
1、基于boost::ucontext函数集开发了协程对象，除此之外无需再引入任何第三方库
2、实现了线程/协程调度器，自带负载均衡，同时支持用户指定CPU核心数和特定CPU核心来运行任务
3、可根据实际需要动态的配置协程栈大小，同时配置了内存池提升多协程的调度速度
4、将复杂的异步处理流程隐藏在框架底层，通过类似go协程的的minico::co_go()完成协程接口封装，上层用户可以使用业务同步开发的方式获得异步的高性能，避免了异步回调和回调分离引发的代码支离破碎
5、对socket进行了类hook处理，用户可以使用和系统socket相同的接口来开发自己的网络服务器，同时项目内也提供了非阻塞模式下协程TCP/RPC服务器实现
6、用户通过继承Service类并实现自己的服务接口，在RPC服务器中添加服务后，即可实现相应的服务
开发服务端程序的一个基本任务是处理并发连接，现在服务端网络编程处理并发连接主要有两种方式：

使用
项目采用CMake进行编译,在工程主目录下新建build目录，进入build目录

cmake ..
make  
编译完成后，TCP和RPC服务器可执行程序位于bin中

以echo服务端为例，

void handleClient(TcpConnection::Ptr conn){
	conn->setTcpNoDelay(true);
	Buffer::Ptr buffer = std::make_shared<Buffer>();
	while (conn->read(buffer) > 0) {
		conn->write(buffer);
	}

	conn->close();
}


int main(int args, char* argv[]) {
	if (args != 2) {
		printf("Usage: %s threads\n", argv[0]);
		return 0;
	}
	Logger::setLogLevel(LogLevel::INFO);
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	IpAddress listen_addr(5000);
	int threads_num = std::atoi(argv[1]);

	Scheduler scheduler(threads_num);
	scheduler.startAsync();
	TcpServer server(listen_addr, &scheduler);
	server.setConnectionHandler(handleClient);
	server.start();

	scheduler.wait();
	return 0;
}
只需要为TcpServer设置连接处理函数，在连接处理函数中，参数TcpConnection::Ptr conn代表此次连接，可以像阻塞IO一样进行读写，如果发生阻塞，当前协程会被切出去，直到可读或者可写事件到来时，该协程会被重新执行。
