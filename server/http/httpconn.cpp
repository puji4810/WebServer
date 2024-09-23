#include "httpconn.h"

HttpConn::HttpConn()
{
	sockfd = -1;
}

void HttpConn::init(int fd)
{
	sockfd = fd;
	request = HttpRequest();
	response = HttpResponse();
	readBuffer = Buffer(4096);
	readBuffer.retrieveAll();
	writeBuffer = Buffer(4096);
	writeBuffer.retrieveAll();
}

bool HttpConn::read()
{
	int saveErrno = 0;
	ssize_t bytesRead = readBuffer.readFd(sockfd, &saveErrno);

	if(bytesRead <= 0){
		return false;
	}

	if(!request.parse(readBuffer.retrieveAllAsString())){
		response.setStatusCode(HttpResponse::HttpsStatus::BAD_REQUEST);
		return false;
	}
	return true;
}

bool HttpConn::write()
{
	response.setBody("<html><body><h1>Hello, World!</h1></body></html>");
	response.setHeader("Content-Type", "text/html");
	if(request.isKeepAlive()){
		response.setKeepAlive(true);
	}
	else{
		response.setKeepAlive(false);
	}

	writeBuffer.append(response.toString());

	int saveErrno = 0;
	ssize_t bytesWrite = writeBuffer.writeFd(sockfd, &saveErrno);

	if(bytesWrite <= 0){
		return false;
	}

	return true;
}

void HttpConn::closeconn()
{
	close(sockfd);
	sockfd = -1;
}

HttpConn::~HttpConn()
{
	if (sockfd != -1)
	{
		close(sockfd);
	}
}

