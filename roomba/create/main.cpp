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

void* CreateUDPListener(void* arg)
{
	return (void*) create->RunUDPListener();
}

void StartListening(Packet & packet)
{
	// initialize udp sender for create
	int remoteSock;
	struct sockaddr_in remoteCreate;
	remoteSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (remoteSock < 0) printf("ERROR: socket\n");
	remoteCreate.sin_family = AF_INET;
	remoteCreate.sin_addr.s_addr = packet.addr.s_addr;
	remoteCreate.sin_port = htons(CREATE_PORT);

	create = new Create(remoteSock, (struct sockaddr*) &remoteCreate, (unsigned long) packet.addr.s_addr);

	pthread_t createSerialThread;
	printf("iRobot Create SerialListner Thread: %d.\n", 
		pthread_create(&createSerialThread, NULL, CreateSerialListener, NULL));
	
	pthread_t createUDPThread;
	printf("iRobot Create UDPListner Thread: %d.\n", 
		pthread_create(&createUDPThread, NULL, CreateSerialListener, NULL));
}

int main(int argc, char *argv[])
{
	// loading input parameters
	if (argc < 2)
	{
		fprintf(stderr,"ERROR, no port provided\n");
		exit(0);
	}
	
	int sock, bufLength;
	socklen_t serverlen, fromlen;
	struct sockaddr_in server;
	struct sockaddr_in from;
	char buf[MAXPACKETSIZE];
	Packet packet;

	// initialize udp listener
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) printf("ERROR: Opening socket\n");
	serverlen = sizeof(server);
	bzero(&server, serverlen);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi((char*)argv+1));
	if (bind(sock, (struct sockaddr *)&server, serverlen) < 0) 
		printf("ERROR: binding\n");
	fromlen = sizeof(struct sockaddr_in);

	printf("Waiting for INIT message ...\n");
	while(1)
	{
		bzero(&buf, sizeof(buf));
		bzero(&packet, sizeof(Packet));
		packet.type = UNKNOWN;
		bufLength = recvfrom(sock, buf, MAXPACKETSIZE, 
				0, (struct sockaddr *)&from, &fromlen);
		if (bufLength < 0) printf("ERROR: recvfrom\n");

		memcpy((unsigned char*)&packet, buf, 256);
		packet.addr = from.sin_addr;
		packet.port = from.sin_port;
		if (packet.type == INIT)
		{
			StartListening(packet);
		}
		if (packet.type == END) 
		{
			printf("Waiting for INIT message ...\n");
		}
		if (packet.type == SHUTDOWN) 
		{
			create->isEnding = true;
			break;
		}
	}

	return 0;
}
