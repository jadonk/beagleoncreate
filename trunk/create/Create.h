#ifndef CREATE_H
#define CREATE_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "pthread.h"

/*! \file Create.h */

/*! The udp port number for direct control of the iRobot Create. */
#define CREATE_PORT 8888

class Create
{
public:
	Create(unsigned long connectedHost);
	~Create();
	
	int InitSerial();
	void CloseSerial();
	void SendSerial(char* buf, int bufLength);
	int RunSerialListener();
	int RunUDPListener(int & sock);
	
	/*! Flag to get the Create class ready to quit. */
	bool isEnding;

private:
	int _fd;
	int _sock;
	unsigned long _connectedHost;

	pthread_mutex_t _serialMutex;
	
	int InitUDPListener();
};

#endif
