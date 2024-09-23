#include "httprequest.h"

HttpRequest::Version HttpRequest::getVersion(std::string version) const
{
	if(version == "HTTP/1.0"){
		return Version::HTTP_10;
	}
	else if(version == "HTTP/1.1"){
		return Version::HTTP_11;
	}
	else{
		return Version::UNKNOWN;
	}
}

bool HttpRequest::parse(const std::string &request){
	size_t pos = 0;
	size_t line_end = request.find("\r\n", pos);

	std::string request_line = request.substr(pos, line_end - pos);
	pos = line_end + 2;

	std::istringstream request_stream(request_line);
	std::string method, path, version;
	request_stream >> method >> path >> version;

	if(method == "GET"){
		this->method = Method::GET;
	}
	else if(method == "POST"){
		this->method = Method::POST;
	}
	else if(method == "HEAD"){
		this->method = Method::HEAD;
	}
	else if(method == "PUT"){
		this->method = Method::PUT;
	}
	else if(method == "DELETE"){
		this->method = Method::DELETE;
	}
	else if(method == "TRACE"){
		this->method = Method::TRACE;
	}
	else if(method == "OPTIONS"){
		this->method = Method::OPTIONS;
	}
	else if(method == "CONNECT"){
		this->method = Method::CONNECT;
	}
	else if(method == "PATCH"){
		this->method = Method::PATCH;
	}
	else{
		this->method = Method::UNKNOWN;
	}

	this->path = path;
	if(version == "HTTP/1.0"){
		this->version = Version::HTTP_10;
	}
	else if(version == "HTTP/1.1"){
		this->version = Version::HTTP_11;
	}
	else{
		this->version = Version::UNKNOWN;
	}

	return parseHeadersAndBody(request, pos);
}

bool HttpRequest::parseHeadersAndBody(const std::string &request, size_t pos)
{
	size_t line_end;

	while((line_end = request.find("\r\n",pos)) != std::string::npos){
		std::string line = request.substr(pos, line_end - pos);
		pos = line_end + 2;
		
		if(line.empty()){
			break;
		}
		size_t colon_pos = line.find(":");
		if(colon_pos != std::string::npos){
			std::string key = line.substr(0, colon_pos);
			std::string value = line.substr(colon_pos + 2);//跳过冒号和空格
			value.erase(value.find_last_not_of("\r\n") + 1);
			headers[key] = value;
		}
	}
	if (this->method == Method::POST)
	{
		this->body = request.substr(pos);
	}
	return true;
}

bool HttpRequest::isKeepAlive() const
{
	auto it = headers.find("Connection");
	if(it != headers.end()){
		if (it->second == "keep-alive" || (version == getVersion("HTTP/1.1") && it->second != "close"))
		{
			return true;
		}
	}
	return false;
}

//void 