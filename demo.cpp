#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

int main()
{
	int server_fd, client_fd;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	// 创建监听套接字
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == 0)
	{
		std::cerr << "Socket failed" << std::endl;
		return -1;
	}

	// 设置监听地址和端口
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(8080);

	// 绑定端口
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		std::cerr << "Bind failed" << std::endl;
		return -1;
	}

	// 开始监听
	if (listen(server_fd, 3) < 0)
	{
		std::cerr << "Listen failed" << std::endl;
		return -1;
	}

	std::cout << "Listening on 127.0.0.1:8080..." << std::endl;

	while (true)
	{
		// 接受客户端连接
		client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
		if (client_fd < 0)
		{
			std::cerr << "Accept failed" << std::endl;
			return -1;
		}

		// 读取客户端请求
		char buffer[30000] = {0};
		read(client_fd, buffer, 30000);
		std::cout << "Received request:\n"
				  << buffer << std::endl;

		// 简单的 HTTP 响应
		const char *response =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Connection: close\r\n"
			"\r\n"
			"<html><body><h1>Hello, World!</h1></body></html>";

		// 发送响应
		write(client_fd, response, strlen(response));

		// 关闭连接
		close(client_fd);
	}

	return 0;
}