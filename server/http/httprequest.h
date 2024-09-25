#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H
#include <string>
#include <unordered_map>
#include <sstream>
#include "../loger/log.h"
class HttpRequest
{
private:
	std::string method;
	std::string version;
	std::string path;
	std::unordered_map<std::string, std::string> headers;
	std::string body;

public:
	HttpRequest() : method("UNKNOWN"), version("UNKNOWN") {}

	bool parse(const std::string &request);
	bool parseHeadersAndBody(const std::string &request, size_t pos);
	bool isKeepAlive() const;
	std::string getMethod() const { return method; }
	std::string getVersion() const { return version; }
	const std::string &getPath() const { return path; }
	const std::unordered_map<std::string, std::string> getHeader() const { return headers; }
	const std::string &getBody() const { return body; }
};

#endif
