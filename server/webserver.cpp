#include "webserver.h"
#define MAX_FD 65536
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
	Log::getInstance().init("./log/webserver.log", LogLevel::DEBUG, 4096);
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
	threadpool->stop();
	timeheap->clear();
	clients.clear();
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

	ret = listen(listen_fd, 10024);
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

		// timeheap->tick();

		for (int i = 0; i < ret; i++)
		{
			int fd = epoller->events[i].data.fd;
			uint32_t events = epoller->events[i].events; // 获取事件类型

			if (fd == listen_fd)
			{
				//handleListen();
				threadpool->submit(&Webserver::handleListen, this);
			}
			else if (events & EPOLLIN)
			{
				{
					std::lock_guard<std::mutex> lock(clients_mutex);
					if (clients.count(fd) == 0)
					{
						LOG_ERROR("fd %d not found in clients", fd);
						continue;
					}
				}
				threadpool->submit(&Webserver::handleRequest, this, fd);
			}
			else if (events & EPOLLOUT)
			{
				{
					std::lock_guard<std::mutex> lock(clients_mutex);
					if (clients.count(fd) == 0)
					{
						LOG_ERROR("fd %d not found in clients", fd);
						continue;
					}
				}
				threadpool->submit(&Webserver::handleResponse, this, fd);
			}
		}
	}
}

void Webserver::handleListen()
{
	// 监听到新的客户端连接，处理 accept
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	int conn_fd = accept(listen_fd, (struct sockaddr *)&addr, &len);

	if (conn_fd > 0)
	{
		setnonblocking(conn_fd);

		{
			std::lock_guard<std::mutex> lock(clients_mutex);
			clients[conn_fd].init(conn_fd);
		}

		epoller->addfd(conn_fd, EPOLLIN | EPOLLET | EPOLLONESHOT);
		LOG_INFO("New connection accepted, fd: %d", conn_fd);
		// 设置超时定时器，60秒后如果连接未活动则关闭连接
		// timeheap->addTimer(conn_fd,std::chrono::milliseconds(60000), [this, conn_fd](int)
		// 				   {
		// 	std::lock_guard<std::mutex> lock(clients_mutex);
		// 	if (clients.find(conn_fd) != clients.end()) {
		// 		LOG_INFO("Connection timeout, closing fd: %d", conn_fd);
		// 		closeConn(conn_fd);
		// 	} });
	}
	else
	{
		std::cerr << "accept error" << std::endl;
		LOG_ERROR("accept error");
	}
}

void Webserver::handleRequest(int fd)
{
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        auto it = clients.find(fd);
        if (it == clients.end())
        {
            return;
        }
        if (!it->second.read())
        {
            LOG_ERROR("httpconn read error, errno: %d", errno);
            return;
        }
        // 继续操作
    }

	// timeheap->addTimer(fd, std::chrono::milliseconds(60000), [this, fd](int)
	// 				   {
	// 		std::lock_guard<std::mutex> lock(clients_mutex);
	// 		if (clients.find(fd) != clients.end()) {
	// 			LOG_INFO("Connection timeout, closing fd: %d", fd);
	// 			closeConn(fd);
	// 		} });

	epoller->modfd(fd, EPOLLOUT | EPOLLET | EPOLLONESHOT);
}


void Webserver::handleResponse(int fd)
{
	if (fd < 0 || fd > MAX_FD)
	{
		return;
	}
	
	{
		std::lock_guard<std::mutex> lock(clients_mutex); 
		// 写入响应数据
		auto it = clients.find(fd);
		if (it == clients.end())
		{
			return;
		}

		if (!it->second.write())
		{
			LOG_ERROR("httpconn write error, errno: %d", errno);
			return;
		}

		// 如果是长连接，继续监听读事件，否则关闭连接
		if (it->second.isKeepAlive())
		{
			epoller->modfd(fd, EPOLLIN | EPOLLET | EPOLLONESHOT);  // 继续监听读事件

			// timeheap->addTimer(fd, std::chrono::milliseconds(5000), [this, fd](int)
			// 				   {
			// std::lock_guard<std::mutex> lock(clients_mutex);
			// if (clients.find(fd) != clients.end()) {
			// 	LOG_INFO("Connection timeout, closing fd: %d", fd);
			// 	closeConn(fd);
			// } });
		}
		else
		{
			it->second.closeconn(); // 确保关闭连接
			clients.erase(it); // 使用迭代器安全地删除
		}
	}
}

void Webserver::closeConn(int fd)
{
	if (fd < 0)
	{
		return;
	}

	std::lock_guard<std::mutex> lock(clients_mutex);
    auto it = clients.find(fd);
    if (it != clients.end()) {
        LOG_INFO("Connection close, fd: %d", fd);
        it->second.closeconn(); // 确保关闭连接
		epoller->delfd(fd);
		timeheap->removeTimer(fd);
		clients.erase(it);      // 使用迭代器安全地删除
    }
}