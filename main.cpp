#include "config.h"
#include <iostream>

int main(int argc, char *argv[])
{
	std::string user = "puji";
	std::string passwd = "1";
	std::string databasename = "yourdb";
	config server_config;
	server_config.parse_arg(argc, argv);
}