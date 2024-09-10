#include "webserver.h"

struct config{
public:
	config();
	~config() = default;

	void parse_arg(int argc, char *argv[]);

	int PORT;
	int thread_num;
};