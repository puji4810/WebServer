#include "webserver.h"

Webserver::Webserver(int port, std::string user, std::string password, std::string databasename, bool opt_mode)
{
	this->port = port;
	this->user = user;
	this->password = password;
	this->databasename = databasename;
	this->opt_mode = opt_mode;
	initsocket();
}

void Webserver::initsocket(){
	struct sockaddr_in addr;
	if(port<1024||port>65535){
		std::cerr<<"port error"<<std::endl;
		exit(1);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd<0){
		std::cerr<<"socket error"<<std::endl;
		exit(1);
	}

	struct linger opt_linger ={0 , 0};
	if(opt_mode){
		opt_linger.l_onoff = 1;
		opt_linger.l_linger = 10;
	}
	int ret = setsockopt(listen_fd, SOL_SOCKET, SO_LINGER, &opt_linger, sizeof(opt_linger));
	if(ret<0){
		std::cerr<<"setsockopt error"<<std::endl;
		exit(1);
	}

	ret = bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
	if(ret<0){
		std::cerr<<"bind error"<<std::endl;
		exit(1);
	}

	ret = listen(listen_fd, 10);
	if(ret<0){
		std::cerr<<"listen error"<<std::endl;
		exit(1);
	}

}