#ifndef CREATE_H
#define CREATE_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define CREATE_PORT 8888

class Create
{
public:
	Create(int sock, struct sockaddr_in & createPort, unsigned long connectedHost);
	~Create();
	
	int InitSerial();
	void CloseSerial();
	void SendSerial(char* buf, int bufLength);
	int RunSerialListener();
	int RunUDPListener();
	
	bool isEnding;

private:
	int _fd;
	int _sock;
	struct sockaddr_in _createPort;
	unsigned long _connectedHost;
};

#endif
