#ifndef ALL_H
#define ALL_H

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <termios.h>
#include <fcntl.h>
#include <poll.h>

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void debugMsg(const char *func, const char *msg)
{
	printf("[%s\t] %s\n", func, msg);
}

#endif
