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
#include "./loger/log.h"
#include "./timer/time_heap.hpp"
#include "./http/httpconn.h"	

struct Webserver{
	Webserver(int port,bool opt_mode);
	~Webserver() = default;
	void start();
private:
	int port;
	int listen_fd;
	int epfd;
	bool opt_mode;
	struct sockaddr_in serv_addr;
	std::unique_ptr<Epoller> epoller;
	std::unique_ptr<ThreadPool> threadpool;
	std::unique_ptr<TimeHeap> timeheap;
	std::unordered_map<int, HttpConn> clients;
	// std::unordered_map<int, HttpConn> users;
	// std::unordered_map<int, sockaddr_in> users;
	int client_fd;
	bool initsocket();
	void eventListen();
	void eventLoop();
	void handleConnect();
	void handleRequest(int fd);
	void handleResponse(int fd);
	void closeConn(int fd);
	std::mutex clients_mutex;
	int setnonblocking(int fd);
	Log& log = Log::getInstance();
	Buffer buf;
};

#endif