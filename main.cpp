#include "server/webserver.h"
#include <signal.h>

int main(int argc, char *argv[])
{
	signal(SIGPIPE, SIG_IGN); // 忽略 SIGPIPE 信号
	int PORT = 8019;
	if(argc == 2){
		PORT = atoi(argv[1]);
	}
	Webserver webserver(PORT, true);
	std::cout << "please visit http://127.0.0.1:" << PORT << std::endl;
	webserver.start();
}