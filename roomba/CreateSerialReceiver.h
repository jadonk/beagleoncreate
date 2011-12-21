#ifndef CREATESERIALRECEIVER_H
#define CREATESERIALRECEIVER_H
#include "all.h"
#include "thread.h"

class CreateSerialReceiver: public PThread
{
	public:
	CreateSerialReceiver(int fd, int sock, struct sockaddr_in * remote);

	virtual void Run();

	bool isEnding;

	private:
	int _fd;
	int _sock;
	struct sockaddr_in * _remote;
};

#endif

