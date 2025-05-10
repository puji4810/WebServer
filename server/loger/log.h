#ifndef LOG_H
#define LOG_H
#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h> // vastart va_end
#include <assert.h>
#include <sys/stat.h> //mkdir
#include "blockqueue.hpp"
// #include "../buffer/buffer.h"
#include "../buffer/modern_buffer.hpp"
#include <memory>
#include <atomic>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

enum LogLevel
{
	DEBUG,
	INFO,
	WARN,
	ERROR,
	FATAL
};

class Log{
public:
	static Log& getInstance(){
		static Log instance;
		return instance;
	}
	bool running() { return !stopLogging; }
	bool init(const std::string &filePath, LogLevel level = INFO, size_t maxQueueSize = 1024);
	void stop();
	void log(LogLevel level, std::string msg);
	void debug(const std::string &msg) { log(DEBUG, msg); }
	void info(const std::string &msg) {log(INFO, msg);}
	void warn(const std::string &msg) {log(WARN, msg);}
	void error(const std::string &msg) {log(ERROR, msg);}

	Log():stopLogging(false),level(INFO) {}
	~Log() { stop(); }
	void asyncWriteLog();
	std::string formatLog(LogLevel level, const std::string &msg);
	std::string getTime();
	std::string getLevel(LogLevel level);
	LogLevel getCurLevel() const { return level; }

private:
	std::string filePath;
	LogLevel level;
	std::atomic<bool> stopLogging;
	std::unique_ptr<BlockQueue<std::string>> logQueue;
	std::thread logthread;
	Buffer buffer;
};

#define LOG_BASE(level, format, ...)                     \
	do                                                   \
	{                                                    \
		Log &log = Log::getInstance();                   \
		if (log.running() && log.getCurLevel() <= level) \
		{                                                \
			char buf[1024];                              \
			snprintf(buf, 1024, format, ##__VA_ARGS__);  \
			log.log(level, buf);                         \
		}                                                \
	} while (0)

#define LOG_DEBUG(format, ...) LOG_BASE(DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) LOG_BASE(INFO, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) LOG_BASE(WARN, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) LOG_BASE(ERROR, format, ##__VA_ARGS__)

#endif