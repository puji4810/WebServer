#include "log.h"

bool Log::init(const std::string &filePath, LogLevel level, size_t maxQueueSize){
	std::filesystem::path logDir = std::filesystem::path(filePath).parent_path();
	// 检查目录是否存在，不存在则创建
	if (!std::filesystem::exists(logDir))
	{
		std::filesystem::create_directories(logDir);
	}
	std::ofstream logFile(filePath, std::ios::out | std::ios::app);
	if (!logFile.is_open())
	{
		std::cerr << "Failed to open log file: " << filePath << std::endl;
		return false;
	}
	logFile.close(); // 文件可以正常打开，关闭，后续异步写入

	buffer.retrieveAll();
	stopLogging = false;
	this->level = level;
	this->filePath = filePath;
	logQueue = std::make_unique<BlockQueue<std::string>>(maxQueueSize);
	logthread = std::thread(&Log::asyncWriteLog, this);
	return true;
}

void Log::stop(){
	stopLogging = true;
	logQueue->stop();
	if (logthread.joinable())
	{
		logthread.join();
	}
}

void Log::log(LogLevel level, std::string msg){
	if (level >= this->level)
	{
		buffer.append(formatLog(level, msg).c_str(), formatLog(level, msg).size());
		logQueue->push(buffer.retrieveAllAsString());
	}
}

void Log::asyncWriteLog()
{
	std::ofstream logFile(filePath, std::ios::out | std::ios::app);
	if (!logFile.is_open())
	{
		std::cerr << "Failed to open log file: " << filePath << std::endl;
		return;
	}
	while (!stopLogging)
	{
		std::string log = logQueue->pop();
		logFile << log << std::endl;
	}
	logFile.close();
}

std::string Log::formatLog(LogLevel level, const std::string &msg){
	std::stringstream ss;
	ss << '[' << getTime() << ']' << '[' << getLevel(level) << ']' << msg;
	return ss.str();
}

std::string Log::getTime(){
	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	char timeBuffer[26];
	ctime_r(&now_time, timeBuffer); // 转换为字符串形式
	timeBuffer[24] = '\0';			// 去掉换行符
	return std::string(timeBuffer);
}

std::string Log::getLevel(LogLevel level){
	switch (level)
	{
	case DEBUG:
		return "DEBUG";
	case INFO:
		return "INFO";
	case WARN:
		return "WARN";
	case ERROR:
		return "ERROR";
	case FATAL:
		return "FATAL";
	default:
		return "UNKNOWN";
	}
}