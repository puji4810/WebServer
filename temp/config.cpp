#include "config.h"

config::config()
{
	PORT = 8080;
}


void config::parse_arg(int argc,char* argv[]){
	while(getopt(argc,argv,"p:t:")!=-1){
		switch(optopt){
			case 'p':{
				PORT = atoi(optarg);
				break;
			}
			case 't':{
				thread_num = atoi(optarg);
				break;
			}
			default:
				break;
		}
	}
}