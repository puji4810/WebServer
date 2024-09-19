#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>
#include <string.h>
#include <iostream>
#include <memory>
#include "epoller.h"
#include "thread_pool.hpp"

struct Webserver{
	Webserver(int port, std::string user,std::string password,std::string databasename,
		bool opt_mode);
	~Webserver();
	void start();
private:
	int port;
	int listen_fd;
	int epfd;
	bool opt_mode;
	std::string user;
	std::string password;
	std::string databasename;
	struct sockaddr_in serv_addr;
	std::unique_ptr<Epoller> epoller;
	std::unique_ptr<ThreadPool> threadpool;
	//std::unordered_map<int, HttpConn> users;
	//std::unordered_map<int, sockaddr_in> users;
	int client_fd;
	bool initsocket();
	void eventListen();
	void eventLoop();
	//void acceptConn();
	//void closeConn(int fd);
	//void addClient(int fd, sockaddr_in addr);
	void handleRequest(int fd);
	int setnonblocking(int fd);

};

#endif