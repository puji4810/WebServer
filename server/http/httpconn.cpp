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
	ssize_t bytesRead;
	{
		//std::lock_guard<std::mutex> lock{HttpConn_mutex};
		bytesRead = readBuffer.readFd(sockfd, &saveErrno);
	}

	if (bytesRead <= 0)
	{
		return false;
	}
	if (!request.parse(readBuffer.retrieveAllAsString()))
	{
		response.setStatusCode(HttpResponse::HttpsStatus::BAD_REQUEST);
		return false;
	}
	return true;
}

bool HttpConn::write()
{
	Buffer filebuf;
	std::string path = request.getPath();
	// 如果请求的是根目录，加载 index.html
	if (path == "/")
	{
		path = "/index.html";
	}
	std::string filepath = "../resources" + path; // 拼接文件路径

	// 如果文件存在，读取文件内容
	if (filebuf.readFile(filepath))
	{
		std::string content = filebuf.retrieveAllAsString();
		response.setStatusCode(HttpResponse::HttpsStatus::OK);				  // 设置状态码为 200 OK
		response.setBody(content);											  // 将文件内容作为响应体
		response.setHeader("Content-Type", "text/html");					  // 设置 Content-Type
		response.setHeader("Content-Length", std::to_string(content.size())); // 设置 Content-Length
	}
	else
	{
		// 如果文件不存在，返回 404 页面
		response.setStatusCode(HttpResponse::HttpsStatus::NOT_FOUND);
		response.setBody("<html><body><h1>404 Not Found</h1></body></html>"); // 设置 404 页面
		response.setHeader("Content-Type", "text/html");
	}
	// 设置保持连接
	if (request.isKeepAlive())
	{
		response.setKeepAlive(true);
	}
	else
	{
		response.setKeepAlive(false);
	}
	// 将响应写入 writeBuffer
	writeBuffer.append(response.toString());
	// 现在将缓冲区的数据写入客户端
	int saveErrno = 0;
	ssize_t bytesWrite;
	{
		//std::lock_guard<std::mutex> lock{HttpConn_mutex};
		if(sockfd<=0){
			return false;
		}
		bytesWrite = writeBuffer.writeFd(sockfd, &saveErrno);
	}
	if (bytesWrite <= 0)
	{
		return false; // 如果写入失败，返回 false
	}
	
	//LOG_INFO("httpconn write success");
	return true; // 写入成功，返回 true
}

void HttpConn::closeconn()
{
	if (isFdclosed == false)
	{
		close(sockfd);
		isFdclosed = true;
		sockfd = -1;
		LOG_INFO("Connection close, sockfd: %d", sockfd);
	}
}

void HttpConn::reset()
{
	request.reset();
	response = HttpResponse();
	readBuffer.retrieveAll();
	writeBuffer.retrieveAll();
	isFdclosed = false;
}