#include "server/webserver.h"


int main(int argc, char *argv[])
{
	std::string user = "root";
	std::string password = "1";
	std::string databasename = "yourdb";
	int PORT = 8019;
	Webserver webserver(PORT, user, password, databasename, true);
	webserver.start();
}