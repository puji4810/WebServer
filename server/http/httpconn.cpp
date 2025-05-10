#include "httpconn.h"
#include <unistd.h>
HttpConn::HttpConn() { sockfd = -1; }

void HttpConn::init(int fd) {
    sockfd = fd;
    request = HttpRequest();
    response = HttpResponse();
    readBuffer = Buffer(4096);
    readBuffer.retrieveAll();
    writeBuffer = Buffer(4096);
    writeBuffer.retrieveAll();
}

bool HttpConn::dealRequest() {
    int saveErrno = 0;
    ssize_t bytesRead = readBuffer.readFd(sockfd, &saveErrno);

    std::string res = readBuffer.retrieveAllAsString();

    if (bytesRead <= 0) {
        LOG_ERROR("read error : %d", saveErrno);
        return false;
    }

    // 检查是否包含 \r\n
    if (res.find("\r\n") == std::string::npos) {
        LOG_ERROR("Incomplete request received");
        return false;
    }

    if (!request.parse(res)) {
        LOG_ERROR("request parse error");
        response.setStatusCode(HttpResponse::HttpsStatus::BAD_REQUEST);
        return false;
    }
    return true;
}

bool HttpConn::dealResponse() {
    std::string path = request.getPath();
    // 如果请求的是根目录，加载 index.html
    if (path == "/") {
        path = "/index.html";
    }
    filepath = "./resources" + path; // 拼接文件路径

    if (request.getMethod() == "GET") {
        return handleGET();
    } else if (request.getMethod() == "POST") {
        return handlePOST();
    } else if (request.getMethod() == "PUT") {
        return handlePUT();
    } else if (request.getMethod() == "DELETE") {
        return handleDELETE();
    } else {
        response.setStatusCode(HttpResponse::HttpsStatus::BAD_REQUEST);
        return false;
    }
}

void HttpConn::closeconn() {
    if (isFdclosed == false) {
        close(sockfd);
        isFdclosed = true;
        sockfd = -1;
        LOG_INFO("Connection close, sockfd: %d", sockfd);
    }
}

void HttpConn::reset() {
    request.reset();
    response = HttpResponse();
    readBuffer.retrieveAll();
    writeBuffer.retrieveAll();
    isFdclosed = false;
}

bool HttpConn::handleGET() {
    Buffer filebuf;
    // 如果文件存在，读取文件内容
    if (filebuf.readFile(filepath)) {
        std::string content = filebuf.retrieveAllAsString();
        response.setStatusCode(HttpResponse::HttpsStatus::OK); // 设置状态码为 200 OK
        response.setBody(content); // 将文件内容作为响应体
        response.setHeader("Content-Type", "text/html"); // 设置 Content-Type
        response.setHeader("Content-Length", std::to_string(content.size())); // 设置 Content-Length
    } else {
        // 如果文件不存在，返回 404 页面
        response.setStatusCode(HttpResponse::HttpsStatus::NOT_FOUND);
        response.setBody("<html><body><h1>404 Not Found</h1></body></html>"); // 设置 404 页面
        response.setHeader("Content-Type", "text/html");
    }
    // 设置保持连接
    if (request.isKeepAlive()) {
        response.setKeepAlive(true);
    } else {
        response.setKeepAlive(false);
    }
    // 将响应写入 writeBuffer
    writeBuffer.append(response.toString());
    // 现在将缓冲区的数据写入客户端
    int saveErrno = 0;
    ssize_t bytesWrite;
    {
        // std::lock_guard<std::mutex> lock{HttpConn_mutex};
        if (sockfd <= 0) {
            return false;
        }
        bytesWrite = writeBuffer.writeFd(sockfd, &saveErrno);
    }
    if (bytesWrite <= 0) {
        return false; // 如果写入失败，返回 false
    }
    // LOG_INFO("httpconn write success");
    return true; // 写入成功，返回 true
}

bool HttpConn::handlePOST() {
    // 从请求中获取 body
    std::string requestBody = request.getBody();

    // 设置响应状态码为 200 OK
    response.setStatusCode(HttpResponse::HttpsStatus::OK);

    // 构建简单的响应消息
    std::string responseBody = "<html><body><h1>POST Request Processed</h1>";
    responseBody += "<p>Your request has been successfully processed.</p>";
    if (!requestBody.empty()) {
        responseBody += "<p>Received data: " + requestBody + "</p>";
    }
    responseBody += "</body></html>";

    // 设置响应体和响应头
    response.setBody(responseBody);
    response.setHeader("Content-Type", "text/html");
    response.setHeader("Content-Length", std::to_string(responseBody.size()));

    // 设置保持连接
    if (request.isKeepAlive()) {
        response.setKeepAlive(true);
    } else {
        response.setKeepAlive(false);
    }

    // 将响应写入 writeBuffer
    writeBuffer.append(response.toString());

    // 将缓冲区的数据写入客户端
    int saveErrno = 0;
    ssize_t bytesWrite;
    {
        if (sockfd <= 0) {
            return false;
        }
        bytesWrite = writeBuffer.writeFd(sockfd, &saveErrno);
    }

    if (bytesWrite <= 0) {
        return false; // 如果写入失败，返回 false
    }

    return true; // 写入成功，返回 true
}

bool HttpConn::handlePUT() {
    // 获取请求路径和请求体
    std::string path = request.getPath();
    std::string requestBody = request.getBody();

    // 设置响应状态码
    // 如果是创建新资源，应该返回 201 Created，但这里简化为 200 OK
    response.setStatusCode(HttpResponse::HttpsStatus::OK);

    // 构建响应消息
    std::string responseBody = "<html><body><h1>PUT Request Processed</h1>";
    responseBody += "<p>Resource at '" + path + "' has been updated.</p>";
    if (!requestBody.empty()) {
        responseBody += "<p>Updated with data: " + requestBody + "</p>";
    }
    responseBody += "</body></html>";

    // 设置响应体和响应头
    response.setBody(responseBody);
    response.setHeader("Content-Type", "text/html");
    response.setHeader("Content-Length", std::to_string(responseBody.size()));

    // 设置保持连接
    if (request.isKeepAlive()) {
        response.setKeepAlive(true);
    } else {
        response.setKeepAlive(false);
    }

    // 将响应写入 writeBuffer
    writeBuffer.append(response.toString());

    // 将缓冲区的数据写入客户端
    int saveErrno = 0;
    ssize_t bytesWrite;
    {
        if (sockfd <= 0) {
            return false;
        }
        bytesWrite = writeBuffer.writeFd(sockfd, &saveErrno);
    }

    if (bytesWrite <= 0) {
        return false; // 如果写入失败，返回 false
    }

    return true; // 写入成功，返回 true
}

bool HttpConn::handleDELETE() {
    // 获取请求路径
    std::string path = request.getPath();

    // 设置响应状态码
    // 对于 DELETE 请求，如果资源成功删除，通常返回 204 No Content
    // 但这里为了简单起见，返回 200 OK 并包含一个响应体
    response.setStatusCode(HttpResponse::HttpsStatus::OK);

    // 构建响应消息
    std::string responseBody = "<html><body><h1>DELETE Request Processed</h1>";
    responseBody += "<p>Resource at '" + path + "' has been deleted.</p>";
    responseBody += "</body></html>";

    // 设置响应体和响应头
    response.setBody(responseBody);
    response.setHeader("Content-Type", "text/html");
    response.setHeader("Content-Length", std::to_string(responseBody.size()));

    // 设置保持连接
    if (request.isKeepAlive()) {
        response.setKeepAlive(true);
    } else {
        response.setKeepAlive(false);
    }

    // 将响应写入 writeBuffer
    writeBuffer.append(response.toString());

    // 将缓冲区的数据写入客户端
    int saveErrno = 0;
    ssize_t bytesWrite;
    {
        if (sockfd <= 0) {
            return false;
        }
        bytesWrite = writeBuffer.writeFd(sockfd, &saveErrno);
    }

    if (bytesWrite <= 0) {
        return false; // 如果写入失败，返回 false
    }

    return true; // 写入成功，返回 true
}
