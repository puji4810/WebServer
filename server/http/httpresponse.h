#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H
#include <string>
#include <unordered_map>	
#include <sstream>
class HttpResponse{
public:
	enum class HttpsStatus
	{
		OK = 200,
		BAD_REQUEST = 400,
		FORBIDDEN = 403,
		NOT_FOUND = 404,
	};

	HttpResponse();

	void setStatusCode(HttpsStatus status);
	void setHeader(const std::string &key, const std::string &value);
	void setBody(const std::string &body);
	void setKeepAlive(bool keepAlive);
	void reset();
	std::string toString()const;

private:
	//std::string version;
	HttpsStatus statusCode;
	std::string body;
	std::string keepAlive;
	std::unordered_map<std::string, std::string> headers;

	std::string getStatusMessage()const;
};

#endif