#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <iostream>

#include "Create.h"
#include "Packet.h"

Create* create;

void* CreateSerialListener(void* arg)
{
	return (void*) create->RunSerialListener();
}

int main()
{
	// initialize udp sender
	int remoteSock;
	struct sockaddr_in remoteCreate;
	remoteSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (remoteSock < 0) printf("ERROR: socket\n");
	remoteCreate.sin_family = AF_INET;
	remoteCreate.sin_addr.s_addr = packet.addr.s_addr;
	remoteCreate.sin_port = htons(CREATE_PORT);

	create = new Create(remoteSock, &remoteCreate);

	pthread_t createSerialThread;
	if (fd != -1)
	{
		printf("iRobot Create SerialListner Thread: %d.\n", 
			pthread_create(&createCallbackThread, NULL, CreateSerialListener, NULL);
	}
	
	create->RunUDPListener();
	return 0;
}