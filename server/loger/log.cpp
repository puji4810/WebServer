#include "log.h"

bool Log::init(const std::string &filePath, LogLevel level, size_t maxQueueSize){
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
	if(level >= this->level){
		logQueue->push(formatLog(level, msg));
	}
}

void Log::asyncWriteLog(){
	std::ofstream logFile(filePath, std::ios::app | std::ios::out);
	if(!logFile){
		std::cerr << "open log file error" << std::endl;
		return;
	}
	while(!stopLogging || !logQueue->empty()){
		std::string msg = logQueue->pop();
		if(!msg.empty()){
			logFile << msg << std::endl;
		}
	}
	logFile.close();
}

std::string Log::formatLog(LogLevel level, const std::string &msg){
	std::stringstream ss;
	ss << '[' << getTime() << ']' << '[' << getLevel(level) << ']' << msg;
	ss << msg;
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
	switch(level){
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
