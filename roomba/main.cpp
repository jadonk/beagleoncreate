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
#include "Packet.h"


bool showDebugMsg = true;

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void debugMsg(const char *func, const char *msg)
{
	if (showDebugMsg)	printf("[%s	] %s\n", func, msg);
}

void MakeConnection(Packet & packet)
{
}

void ProcessPackets(char *buf, int len, Packet & packet)
{
	debugMsg(__func__, "Start processing packet ...");
	memcpy((unsigned char*)&packet, buf, 256);
	switch(packet.type)
	{
		case INIT:
			debugMsg(__func__, "======= packet received, type: INIT");
			break;
		case END:
			debugMsg(__func__, "======= packet received, type: END");
			break;
		case CTRL:
			debugMsg(__func__, "======= packet received, type: CTRL");
			break;
		case DATA:
			debugMsg(__func__, "======= packet received, type: DATA");
			break;
		case ERROR:
			debugMsg(__func__, "======= packet received, type: ERROR");
			break;
		case SHUTDOWN:
			debugMsg(__func__, "======= packet received, type: SHUTDOWN");
			break;
		default:
			debugMsg(__func__, "======= packet received, type: UNKNOWN");
			packet.type = UNKNOWN;
			break;
	}
}

void* ListenMessage(void* arg)
{
	int sock, bufLength;
	socklen_t serverlen, fromlen;
	struct sockaddr_in server;
	struct sockaddr_in from;
	char buf[MAXPACKETSIZE];
	Packet packet;

	// initialize udp listener
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) error("Opening socket");
	serverlen = sizeof(server);
	bzero(&server, serverlen);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi((char*)arg));
	if (bind(sock, (struct sockaddr *)&server, serverlen) < 0) error("binding");
	fromlen = sizeof(struct sockaddr_in);

	debugMsg(__func__, "Waiting for INIT message ...");
	while(1)
	{
		bzero(&buf, sizeof(buf));
		bzero(&packet, sizeof(Packet));
		packet.type = UNKNOWN;
		bufLength = recvfrom(sock, buf, MAXPACKETSIZE, 
				0, (struct sockaddr *)&from, &fromlen);
		if (bufLength < 0) error("recvfrom");
		ProcessPackets(buf, bufLength, packet);
		if (packet.type == END) debugMsg(__func__, "Waiting for INIT message ...");
		if (packet.type == SHUTDOWN) break;
	}

	return 0;
}
void HandleControls()
{
	
}
void StreamSensorData()
{

}

int main(int argc, char *argv[])
{
	// loading input parameters
	if (argc < 2)
	{
		fprintf(stderr,"ERROR, no port provided\n");
		exit(0);
	}
	if (argc > 2)
	{
		showDebugMsg = (strcmp(argv[2],"hideDebug") == 0)?false:showDebugMsg;
	}

	pthread_t listenerThread;
	printf("Listener Thread: %d.\n", 
		pthread_create(&listenerThread, NULL, ListenMessage, (void*)argv[1]));

	pthread_join(listenerThread, NULL);
	// start sensor data thread
/*	StreamSensorData();
	// start message listening thread
	ListenMessage();*/

	// return to idle when receive a finish msg 

	// or restart when receive another init msg

	return 0;
}
