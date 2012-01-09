#ifndef CREATE_H
#define CREATE_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "pthread.h"

#include "Packet.h"

#define CREATE_PORT 8888

class Create
{
public:
	Create(int sock, struct sockaddr_in & createPort, unsigned long connectedHost, pthread_mutex_t & bufMutex);
	~Create();
	
	int InitSerial();
	void CloseSerial();
	void SendSerial();
	int RunSerialListener();
	int RunUDPListener(int & sock);
	
	bool isEnding;

private:
	int _fd;
	int _sock;
	struct sockaddr_in _createPort;
	unsigned long _connectedHost;
	
	pthread_mutex_t _bufMutex;
	int _bufLength;
	char _buf[MAXPACKETSIZE];

	int InitUDPListener();
};

#endif
