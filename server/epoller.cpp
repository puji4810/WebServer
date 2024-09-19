#include "epoller.h"

Epoller::Epoller(int max_event)
{
	// 创建 epoll 实例，并启用 EPOLL_CLOEXEC 标志
	epoll_fd = epoll_create1(EPOLL_CLOEXEC);

	// 检查 epoll_create1 是否成功
	if (epoll_fd == -1)
	{
		throw std::runtime_error("Failed to create epoll instance");
	}

	// 初始化事件容器
	events.resize(max_event);
}

Epoller::~Epoller()
{
	if (epoll_fd != -1)
	{
		close(epoll_fd); // 关闭 epoll 实例，释放资源
	}
}

bool Epoller::addfd(int fd, uint32_t event){
	epoll_event ev;
	ev.data.fd = fd;
	ev.events = event;

	// 添加事件 EPOLL_CTL_ADD
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
	{
		return false;
	}

	return true;
}

bool Epoller::modfd(int fd, uint32_t event){
	epoll_event ev;
	ev.data.fd = fd;
	ev.events = event;

	// 修改事件 EPOLL_CTL_MOD
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1)
	{
		return false;
	}

	return true;
}

bool Epoller::delfd(int fd){
	if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1){
		return false;
	}
	return true;
}

int Epoller::wait(int timeout)
{
	return epoll_wait(epoll_fd, events.data(), static_cast<int>(events.size()), timeout);
}

epoll_event& Epoller::get(int i){
	assert(i >= 0 && i < static_cast<int>(events.size()) && "Index out of bounds");
	return events[i];
}