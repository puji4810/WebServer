#ifndef EPOLLER_H
#define EPOLLER_H
#include <sys/epoll.h> //epoll_ctl()
#include <fcntl.h>	   // fcntl()
#include <unistd.h>	   // close()
#include <assert.h>	   // close()
#include <vector>
#include <errno.h>
#include <stdexcept>

class Epoller{
	friend class Webserver;
public:
	Epoller(int max_event = 1024);
	~Epoller();
	bool addfd(int fd, uint32_t event);
	bool modfd(int fd, uint32_t event);
	bool delfd(int fd);
	int wait(int timeout = -1);
	epoll_event& get(int i);
private:
	int epoll_fd;
	std::vector<epoll_event> events;
};

#endif