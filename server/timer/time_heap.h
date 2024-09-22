#ifndef TIME_HEAP_H
#define TIME_HEAP_H
#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h>
#include <functional>
#include <assert.h>
#include <chrono>
#include <queue>
#include "../loger/log.h"

class TimeHeap;
class Timer
{ // 定时器
	friend class TimeHeap;
public:
	using Clock = std::chrono::steady_clock;
	using TimerCallback = std::function<void(int)>;

	Timer(int fd, Clock::time_point expireTime, TimerCallback cb):
		fd(fd), expireTime(expireTime), cb(cb), valid(true) {}

	Clock::time_point getExpireTime() const { return expireTime; }

	void runCallback() {
		if (valid && cb)
		{
			cb(fd);
		}
	}

	void invalidate() { valid = false; }

	bool isValid() const { return valid; }

private:
	Clock::time_point expireTime;
	TimerCallback cb;
	bool valid;
	int fd;
};

class TimeHeap{//时间堆
public:
	using Clock = std::chrono::steady_clock;
	using TimerCallback = std::function<void(int)>;

	TimeHeap() = default;

	void addTimer(int fd, Clock::time_point expireTime, TimerCallback cb) {
		auto expireTimePoint = expireTime;
	}

private:
	std::priority_queue<Timer*, std::vector<Timer*>, std::greater<Timer*>> timers;
	std::unordered_map<int, Timer *> fdMap_;
};

#endif