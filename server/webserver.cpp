#include "webserver.h"
#define check_ret(ret, msg)            \
	if (ret < 0)                       \
	{                                  \
		std::cerr << msg << std::endl; \
		LOG_ERROR(msg);                   \
		exit(1);                       \
	}
#define check_ret0(ret, msg)           \
	if (ret == 0)                      \
	{                                  \
		std::cerr << msg << std::endl; \
		LOG_ERROR(msg);                \
		exit(1);                       \
	} // 特别检查0的情况

Webserver::Webserver(int port, bool opt_mode):
	port(port), opt_mode(opt_mode)
{
	epoller = std::make_unique<Epoller>();
	threadpool = std::make_unique<ThreadPool>();
	timeheap = std::make_unique<TimeHeap>();
	threadpool->start();
	Log::getInstance().init("./log/webserver.log", LogLevel::DEBUG, 1024);
	if(!initsocket()){
		LOG_ERROR("==========initsocket error==========");
		exit(1);
	}
	LOG_INFO("==========initsocket success==========");
	LOG_INFO("==========webserver start=============");
	LOG_INFO("Port is %d ", port);
	LOG_INFO("please visit 127.0.0.1:%d", port);
}

Webserver::~Webserver(){
	close(listen_fd);
}

void Webserver::start(){
	eventLoop();
}

bool Webserver::initsocket(){
	struct sockaddr_in addr;
	if(port<1024||port>65535){
		std::cerr<<"port error"<<std::endl;
		exit(1);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	check_ret(listen_fd, "socket error");

	struct linger opt_linger ={0 , 0};
	if(opt_mode){
		opt_linger.l_onoff = 1;
		opt_linger.l_linger = 10;
	}//优雅关闭
	int ret = setsockopt(listen_fd, SOL_SOCKET, SO_LINGER, &opt_linger, sizeof(opt_linger));
	check_ret(ret, "setsockopt error");

	ret = bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
	check_ret(ret, "bind error,请检查端口是否被占用");

	ret = listen(listen_fd, 10);
	check_ret(ret, "listen error");

	ret = epoller->addfd(listen_fd, EPOLLIN); // 返回的是bool,0表示失败
	check_ret0(ret, "epoller addfd error");
	setnonblocking(listen_fd);
	std::cout << "server start" << std::endl;
	return true;
}

int Webserver::setnonblocking(int fd){
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

void Webserver::eventLoop()
{
	while (1)
	{
		int ret = epoller->wait(); // 等待事件发生
		if (ret < 0)
		{
			std::cerr << "epoller wait error" << std::endl;
			LOG_ERROR("epoller wait error");
			break;
		}
		for (int i = 0; i < ret; i++)
		{
			int fd = epoller->events[i].data.fd;
			uint32_t events = epoller->events[i].events; // 获取事件类型

			if (fd == listen_fd)
			{
				// 监听到新的客户端连接，处理 accept
				struct sockaddr_in addr;
				socklen_t len = sizeof(addr);
				int conn_fd = accept(listen_fd, (struct sockaddr *)&addr, &len);

				if (conn_fd > 0)
				{
					setnonblocking(conn_fd);
					clients[conn_fd].init(conn_fd);
					// 将新连接的 fd 添加到 epoll 中监听读事件 (EPOLLIN)
					epoller->addfd(conn_fd, EPOLLIN | EPOLLET | EPOLLONESHOT);  // 边缘触发+oneshot模式
					LOG_INFO("New connection accepted, fd: %d", conn_fd);

					timeheap->addTimer(conn_fd, std::chrono::milliseconds(5000), [this, conn_fd](int)
					{closeConn(conn_fd);});
				}
				else
				{
					std::cerr << "accept error" << std::endl;
					LOG_ERROR("accept error");
				}
			}
			else if (events & EPOLLIN)
			{
				// 如果是可读事件，提交请求处理到线程池
				threadpool->submit([this, fd]()
				{
                    handleRequest(fd);
                    // 重置定时器，如果5分钟内没有新的数据交互就超时
					timeheap->addTimer(fd, std::chrono::milliseconds(50000), [this, fd](int)
					{ closeConn(fd); }); });
			}
			else if (events & EPOLLOUT){
				// 如果是可写事件，提交响应写入到线程池
				threadpool->submit([this, fd]()
				{
					handleWrite(fd);
					// 重置定时器，如果5分钟内没有新的数据交互就超时
					timeheap->addTimer(fd, std::chrono::milliseconds(50000), [this, fd](int)
					{ closeConn(fd); }); });
			}
			else if (events & EPOLLRDHUP)
			{
				// 客户端关闭连接，关闭服务器这边的连接
				LOG_INFO("client closed connection");
				close(fd);
			}
			else if (events & EPOLLERR)
			{
				// 处理连接上的错误
				LOG_ERROR("connection error");
				close(fd);
			}
		}
	}
}


void Webserver::handleRequest(int fd)
{
	//std::lock_guard<std::mutex> lock(clients_mutex);
	auto &httpconn = clients[fd];

	if (!httpconn.read())
	{
		LOG_ERROR("httpconn read error, errno: %d", errno);
		httpconn.closeconn();
		return;
	}

	epoller->modfd(fd, EPOLLOUT | EPOLLET | EPOLLONESHOT);
}

void Webserver::handleWrite(int fd)
{
	//std::lock_guard<std::mutex> lock(clients_mutex);
	auto &httpconn = clients[fd];

    // 写入响应数据
    if (!httpconn.write())
    {
		LOG_ERROR("httpconn write error, errno: %d", errno);
		httpconn.closeconn(); // 写入失败，关闭连接
		return;
	}

    // 如果是长连接，继续监听读事件，否则关闭连接
    if (httpconn.isKeepAlive())
    {
        epoller->modfd(fd, EPOLLIN | EPOLLET | EPOLLONESHOT);  // 继续监听读事件
    }
    else
    {
        httpconn.closeconn();  // 非 Keep-Alive 连接，关闭连接
	}
}

void Webserver::closeConn(int fd)
{
	std::lock_guard<std::mutex> lock(clients_mutex);
	if (clients.find(fd) != clients.end())
	{
		LOG_INFO("Connection close, fd: %d", fd);
		clients[fd].closeconn(); // 关闭连接
		clients.erase(fd);		 // 从clients中删除
	}
}