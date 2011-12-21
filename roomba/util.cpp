#include "util.h"

void Util::error(const char *msg)
{
	perror(msg);
	exit(0);
}

void Util::debugMsg(const char *func, const char *msg)
{
	printf("[%s\t] %s\n", func, msg);
}
