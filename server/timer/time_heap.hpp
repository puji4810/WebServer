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

struct TimerCmp
{
	bool operator()(Timer *a, Timer *b) const
	{
		return a->getExpireTime() > b->getExpireTime();
	}
};

class TimeHeap{//时间堆
public:
	std::mutex Timemutex;
	using Clock = std::chrono::steady_clock;
	using TimerCallback = std::function<void(int)>;
	std::mutex timermutex;

	TimeHeap() = default;

	Timer* addTimer(int fd, std::chrono::milliseconds duration, TimerCallback cb)
	{
		std::lock_guard<std::mutex> lock{timermutex};
		auto expireTimePoint = Clock::now() + duration;
		Timer *timer = new Timer(fd, expireTimePoint, cb);
		timers.push(timer);
		fdMap[fd] = timer;
		return timer;
	}

	void removeTimer(int fd){
		std::lock_guard<std::mutex> lock{timermutex};
		auto target = fdMap.find(fd);
		if(target != fdMap.end()){
			target->second->invalidate();
			fdMap.erase(target);
		}
	}

	void tick(){
		std::lock_guard<std::mutex> lock{timermutex};
		while(!timers.empty()){
			auto cur = Clock::now();
			auto timer = timers.top();
			if(!timer->isValid()){//无效定时器(removeTimer)
				timers.pop();
				delete timer;
				continue;
			}
			if(timer->getExpireTime() > cur){
				break;//时间未到
			}
			timer->runCallback();
			timers.pop();
			delete timer;//时间到了，删除定时器
		}
	}

	std::chrono::milliseconds getNextTimeout() const{
		if(timers.empty()){
			return std::chrono::milliseconds(-1);
		}
		auto now = Clock::now();
		auto nextExpireTime = timers.top()->getExpireTime();
		return std::chrono::duration_cast<std::chrono::milliseconds>(nextExpireTime - now);
	}

	void clear(){
		while(!timers.empty()){
			auto timer = timers.top();
			timers.pop();
			delete timer;
		}
		fdMap.clear();
	}

	~TimeHeap(){
		clear();
	}

private:
	std::priority_queue<Timer *, std::vector<Timer *>, TimerCmp> timers;
	std::unordered_map<int, Timer *> fdMap;
};

#endif