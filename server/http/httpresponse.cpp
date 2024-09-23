#include "httpresponse.h"

HttpResponse::HttpResponse()
{
	//version = "HTTP/1.1";
	statusCode = HttpsStatus::OK;
	keepAlive = "close";
}

void HttpResponse::setStatusCode(HttpsStatus status)
{
	statusCode = status;
}

void HttpResponse::setHeader(const std::string &key, const std::string &value)
{
	headers[key] = value;
}

void HttpResponse::setBody(const std::string &body)
{
	this->body = body;
	headers["Content-Length"] = std::to_string(body.size());
}

void HttpResponse::setKeepAlive(bool keepAlive)
{
	this->keepAlive = keepAlive;
	headers["Connection"] = keepAlive ? "keep-alive" : "close";
}

std::string HttpResponse::toString()const
{
	std::stringstream ss;
	ss << "HTTP/1.1 " << static_cast<int>(statusCode) << " " << getStatusMessage() << "\r\n";

	for (const auto &header : headers)
	{
		ss << header.first << ": " << header.second << "\r\n";
	}

	ss << "\r\n";
	ss << body;
	return ss.str();
}

std::string HttpResponse::getStatusMessage()const{
	switch(statusCode){
		case HttpsStatus::OK:
			return "OK";
		case HttpsStatus::BAD_REQUEST:
			return "Bad Request";
		case HttpsStatus::FORBIDDEN:
			return "Forbidden";
		case HttpsStatus::NOT_FOUND:
			return "Not Found";
		default:
			return "Unknown";
	}
}