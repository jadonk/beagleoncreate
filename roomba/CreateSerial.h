#ifndef CREATESERIAL_H
#define CREATESERIAL_H 
#include "all.h"

class CreateSerial
{
	public:
		CreateSerial(unsigned long connectedHost, int remoteSock, struct sockaddr_in * remoteCreate);
		~CreateSerial();
		
		int InitCreateSerial();
		void CloseCreateSerial(int fd);
		void SendSerialToCreate(int fd, char* buf, int bufLength);
		int CreateCallbackHandler(void* arg);
		int CreateSerialHandler(void* arg);

		bool isEnding;
	
	private:
		unsigned long _connectedHost;
		int _remoteSock;
		struct sockaddr_in * _remoteCreate;

};

#endif
