#pragma once
#include <functional>
#include <memory>
#include <vector>

struct EventLoop
{
	EventLoop();
	~EventLoop();
	void loop();
	void quit();
	void 
private:
	bool looping;
};