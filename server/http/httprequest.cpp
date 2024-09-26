#include "httprequest.h"

bool HttpRequest::parse(const std::string &request){

	size_t pos = 0;
	size_t line_end = request.find("\r\n", pos);
	if (line_end == std::string::npos)
	{
		LOG_ERROR("Failed to find end of request line");
		return false;
	}

	std::string request_line = request.substr(pos, line_end - pos);
	//LOG_INFO("Request Line: %s", request_line.c_str());

	pos = line_end + 2;

	std::istringstream request_stream(request_line);
	std::string methodStr, pathStr, versionStr;
	request_stream >> methodStr >> pathStr >> versionStr;

	// 确认解析是否成功
	if (methodStr.empty() || pathStr.empty() || versionStr.empty())
	{
		LOG_ERROR("Failed to parse request line, Method: [%s], Path: [%s], Version: [%s]",
				  methodStr.c_str(), pathStr.c_str(), versionStr.c_str());
		return false;
	}

	// 赋值并打印路径
	this->method = methodStr;
	this->path = pathStr;
	this->version = versionStr;

	return parseHeadersAndBody(request, pos);
}

bool HttpRequest::parseHeadersAndBody(const std::string &request, size_t pos)
{
	size_t line_end;

	while ((line_end = request.find("\r\n", pos)) != std::string::npos)
	{
		std::string line = request.substr(pos, line_end - pos);
		pos = line_end + 2;

		if (line.empty())
		{
			break;
		}
		size_t colon_pos = line.find(":");
		if (colon_pos != std::string::npos)
		{
			std::string key = line.substr(0, colon_pos);
			std::string value = line.substr(colon_pos + 2); // 跳过冒号和空格
			value.erase(value.find_last_not_of("\r\n") + 1);
			headers[key] = value;
		}
	}
	if (this->method == "POST" || this->method == "PUT")
	{
		this->body = request.substr(pos);
	}
	return true;
}

bool HttpRequest::isKeepAlive() const
{
	auto it = headers.find("Connection");
	if (it != headers.end())
	{
		if (it->second == "keep-alive" || (version == "HTTP/1.1" && it->second != "close"))
		{
			return true;
		}
	}
	return false;
}

