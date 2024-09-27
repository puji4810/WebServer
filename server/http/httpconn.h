#ifndef HTTPCONN_H
#define HTTPCONN_H
#include <iostream>
#include <string>
#include <unistd.h> // for close
#include <sys/types.h>
#include <sys/socket.h>
#include "httprequest.h"
#include "httpresponse.h"
#include "../buffer/buffer.h"
#include <sstream>
#include <fstream>
#include "../loger/log.h"

class HttpConn{
public:
	std::mutex HttpConn_mutex;

	HttpConn();
	~HttpConn() = default;

	void init(int fd);
	void reset();
	bool read();
	bool write();
	void closeconn();
	bool isKeepAlive() const { return request.isKeepAlive(); }
	bool isClosed() const { return isFdclosed; }
	int GetFd() const { return sockfd; }
private:
	int sockfd;
	HttpRequest request;
	HttpResponse response;
	Buffer readBuffer;
	Buffer writeBuffer;
	bool isFdclosed = false;
};

#endif