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

Webserver::Webserver(int port, std::string user, std::string password, std::string databasename, bool opt_mode):
	port(port), user(user), password(password), databasename(databasename), opt_mode(opt_mode)
{
	epoller = std::make_unique<Epoller>();
	threadpool = std::make_unique<ThreadPool>();
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
	}
	int ret = setsockopt(listen_fd, SOL_SOCKET, SO_LINGER, &opt_linger, sizeof(opt_linger));
	check_ret(ret, "setsockopt error");

	ret = bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
	check_ret(ret, "bind error,请检查端口是否被占用");

	ret = listen(listen_fd, 10);
	check_ret(ret, "listen error");

	ret = epoller->addfd(listen_fd, EPOLLIN);//返回的是bool,0表示失败
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
					// 提交新连接的处理到线程池
					threadpool->submit(&Webserver::handleRequest, this, conn_fd);
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
				threadpool->submit(&Webserver::handleRequest, this, fd);
			}
		}
	}
}

// void Webserver::acceptConn(){
// 	struct sockaddr_in addr;
// 	socklen_t len = sizeof(addr);
// 	int conn_fd = accept(listen_fd, (struct sockaddr *)&addr, &len);
// 	check_ret(conn_fd, "accept error");
// 	addClient(conn_fd, addr);
// 	int ret = epoller->addfd(conn_fd, EPOLLIN);
// 	check_ret0(ret, "epoller addfd error");
// 	setnonblocking(conn_fd);
// }

// void Webserver::addClient(int fd, sockaddr_in addr){
// 	users[fd] = addr;
// }

void Webserver::handleRequest(int fd)
{
	char buffer[30000] = {0};
	int ret = read(fd, buffer, 30000);
	check_ret(ret, "read error");
	std::cout << "Received request:\n"
			  << buffer << std::endl;

	const char *response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Connection: close\r\n"
		"\r\n"
		"<html><body><h1>Hello, World!</h1></body></html>";

	// 发送响应到客户端
	write(fd, response, strlen(response));

	// 关闭连接
	close(fd);
}
