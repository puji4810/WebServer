#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H
#include <string>
#include <unordered_map>

class HttpRequest{
public:
	enum class Method{
		GET, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH, UNKNOWN
	};

	enum class Version{
		HTTP_10, HTTP_11, UNKNOWN
	};

private:
	Method method;
	Version version;
	std::string path;
	std::unordered_map<std::string, std::string> headers;
	std::string body;

public:
	HttpRequest():method(Method::UNKNOWN), version(Version::UNKNOWN) {}

	bool parse(const std::string &request);
	Method getMethod() const { return method; }
	Version getVersion() const { return version; }
	const std::string& getPath() const { return path; }
	const std::unordered_map<std::string, std::string> getHeader() const{ return headers; };
	const std::string& getBody() const { return body; }
};

#endif