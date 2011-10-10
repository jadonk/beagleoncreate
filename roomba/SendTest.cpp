/* UDP client in the internet domain */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include "Packet.h"

void error(const char *);
int main(int argc, char *argv[])
{
	int sock, n;
	unsigned int length;
	struct sockaddr_in server, from;
	struct hostent *hp;
	char buffer[256];
	struct Packet packet;

	if (argc != 3)
	{
		printf("Usage: server port\n");
		exit(1);
	}	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) error("socket");

	server.sin_family = AF_INET;
	hp = gethostbyname(argv[1]);
	if (hp == 0) error("Unknown host");

	bcopy((char *)hp->h_addr,
		(char *)&server.sin_addr,
		hp->h_length);
	server.sin_port = htons(atoi(argv[2]));
	length = sizeof(struct sockaddr_in);
	while (1)
	{
		printf("Please enter the message: ");
		bzero(buffer, 256);
		std::cin >> buffer;	
		if (strcmp(buffer, "INIT") == 0)
		{
			printf("Sending INIT message ...\n");
			packet.type = INIT;
			memcpy(packet.u.init.data, buffer, 256);
			memcpy(buffer, (unsigned char*)&packet, 256);
		}
		else if (strcmp(buffer, "END") == 0)
		{
			printf("Sending END message ...\n");
			packet.type = END;
			memcpy(packet.u.end.data, buffer, 256);
			memcpy(buffer, (unsigned char*)&packet, 256);
		}
		else if (strcmp(buffer, "CTRL") == 0)
		{
			printf("Sending CTRL message ...\n");
			packet.type = CTRL;
			memcpy(packet.u.ctrl.data, buffer, 256);
			memcpy(buffer, (unsigned char*)&packet, 256);
		}
		else if (strcmp(buffer, "DATA") == 0)
		{
			printf("Sending DATA message ...\n");
			packet.type = DATA;
			memcpy(buffer, (unsigned char*)&packet, 256);
		}
		else if (strcmp(buffer, "ERROR") == 0)
		{
			printf("Sending ERROR message ...\n");
			packet.type = ERROR;
			memcpy(packet.u.error.data, buffer, 256);
			memcpy(buffer, (unsigned char*)&packet, 256);
		}
		else if (strcmp(buffer, "SHUTDOWN") == 0)
		{
			printf("Sending SHUTDOWN message ...\n");
			packet.type = SHUTDOWN;
			memcpy(packet.u.shutdown.data, buffer, 256);
			memcpy(buffer, (unsigned char*)&packet, 256);
		}
		else 
		{
			printf("Sending UNKNOWN message ...\n");
			packet.type = UNKNOWN;
			memcpy(buffer, (unsigned char*)&packet, 256);
		}

		n = sendto(sock, buffer, strlen(buffer), 0, (const struct sockaddr *)&server, length);
		if (n < 0) error("sendto");
	}
	
	close (sock);
	return 0;
}

void error(const char *msg)
{
	perror(msg);
	exit(0);
}
