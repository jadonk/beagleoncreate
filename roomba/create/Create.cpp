#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <termios.h>
#include <fcntl.h>
#include <poll.h>

#include "Create.h"

#define CREATE_SERIAL_PORT "/dev/ttyUSB0"
#define CREATE_SERIAL_BRATE B57600

Create::Create(int sock, struct sockaddr_in & createPort, unsigned long connectedHost, pthread_mutex_t & bufMutex)
{
	_fd = -1;
	_sock = sock;
	_createPort = createPort;
	_connectedHost = connectedHost;
	isEnding = false;
	_bufMutex = bufMutex;
	_bufLength = 0;
	InitSerial();
}

Create::~Create()
{
	CloseSerial();
}

int Create::InitSerial()
{
	_fd = open(CREATE_SERIAL_PORT, O_RDWR | O_NOCTTY | O_NDELAY);

	if(_fd == -1) // if open is unsuccessful
	{
		printf("Unable to open %s.\n", CREATE_SERIAL_PORT);
		return -1;
	}
	else
	{
		fcntl(_fd, F_SETFL, 0);
		printf("Create serial port opened.\n");
	}
	
	// configure port
	struct termios portSettings;
	if (cfsetispeed(&portSettings, CREATE_SERIAL_BRATE) != 0)
		printf("Failed setting cfsetispeed\n");
	if (cfsetospeed(&portSettings, CREATE_SERIAL_BRATE) != 0)
		printf("Failed setting cfsetospeed\n");

	// set no parity, stop bits, databits
	portSettings.c_cflag &= ~PARENB;
	portSettings.c_cflag &= ~CSTOPB;
	portSettings.c_cflag &= ~CSIZE;
	portSettings.c_cflag |= CS8;

	if (tcsetattr(_fd, TCSANOW, &portSettings) != 0)
		printf("Failed pushing portSettings\n");
	
	return _fd;
}

void Create::CloseSerial()
{
	close(_fd);
}

void Create::SendSerial()
{
	int bufLength = 0;
	char buf[MAXPACKETSIZE];
	if (_fd == -1)
	{
		printf("ERROR: _fd is not initialized\n");
		return;
	}

	while(1)
	{
		usleep(100);

		if (isEnding)
			break;
	
		pthread_mutex_lock( &_bufMutex);
		bufLength = _bufLength;
		memcpy(buf, _buf, bufLength);
		_bufLength = 0;
		pthread_mutex_unlock( &_bufMutex);
		if (bufLength == 0)
			continue;

		if (write(_fd, buf, bufLength) == -1)
		{
			printf("ERROR: write error occured.\n");
			return;
		}
		printf("Sending to Create: \n");
		for (int i = 0; i < bufLength; i++)
		{
			printf("%i ", int(buf[i]));
		}
		printf("\n");
	}
}

int Create::RunSerialListener()
{
	char buf[MAXPACKETSIZE];
	int ret;
	int bufLength;
	int            max_fd;
	fd_set         input;
	struct timeval timeout;
	
	if (_fd == -1)
	{
		printf("ERROR: _fd is not initialized\n");
		return -1;
	}

	while(1)
	{
		if(isEnding)
			break;
			
		/* Initialize the input set */
		FD_ZERO(&input);
		FD_SET(_fd, &input);
		max_fd = _fd + 1;
		
		/* Initialize the timeout structure */
		timeout.tv_sec  = 1;
		timeout.tv_usec = 0;

		/* Do the select */
		ret = select(max_fd, &input, NULL, NULL, &timeout);
		//ret = select(max_fd, &input, NULL, NULL, NULL);

		/* See if there was an error */
		if (ret < 0)
			printf("ERROR: select failed\n");
		else if (ret != 0)
		{
			/* We have input */
			if (FD_ISSET(_fd, &input))
			{
				bufLength = read(_fd, buf, MAXPACKETSIZE);
				if (sendto(_sock, buf, bufLength, 0, (const struct sockaddr *) &_createPort, sizeof(struct sockaddr_in)) < 0) printf("ERROR: sendto\n");
				printf("Received from Create: \n");
				for (int i = 0; i < bufLength; i++)
				{	
					printf("%i ", int(buf[i]));
				}
				printf("\n");
			}
		}
		fflush(stdout);
	}
	return 0;
}

int timeout_recvfrom(int sock, void *data, int len, struct sockaddr * sockfrom, socklen_t *fromlen, int timeoutInSec)
{
	fd_set socks;
	struct timeval timeout;
	FD_ZERO(&socks);
	FD_SET(sock, &socks);
	timeout.tv_sec = timeoutInSec;
	if (select(sock + 1, &socks, NULL, NULL, &timeout))
		return recvfrom(sock, data, len, 0, sockfrom, fromlen);
	else
		return 0;
}

int Create::InitUDPListener()
{
	int sock;
	socklen_t serverlen;
	struct sockaddr_in server;
	// initialize udp listener
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) printf("ERROR: Opening socket\n");
	serverlen = sizeof(server);
	bzero(&server, serverlen);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(CREATE_PORT);
	if (bind(sock, (struct sockaddr *)&server, serverlen) < 0) 
		printf("ERROR: binding\n");
	return sock;
}

int Create::RunUDPListener(int & sock)
{
	int len;
	socklen_t fromlen;
	struct sockaddr_in from;
	char buf[MAXPACKETSIZE];

	if (sock == -1)	sock = InitUDPListener();

	fromlen = sizeof(struct sockaddr_in);
	printf("Ready to listen to Create message ...\n");
	while(1)
	{
		if (isEnding)
		{
			printf("RunUDPListener received Ending flag\n");
			break;
		}
		
		bzero(&buf, sizeof(buf));
		//len = recvfrom(sock, buf, MAXPACKETSIZE, 
		//		0, (struct sockaddr *)&from, &fromlen);
		len = timeout_recvfrom(sock, buf, MAXPACKETSIZE, 
				(struct sockaddr *) &from, &fromlen, 1);
		if (len == 0) continue;
		if (len < 0) printf("ERROR: recvfrom\n");
	
		if (_connectedHost != from.sin_addr.s_addr)
			continue;

		pthread_mutex_lock( &_bufMutex);	
		memcpy(_buf + _bufLength, buf, len);
		_bufLength += len;
		pthread_mutex_unlock( &_bufMutex);
		//SendSerial(buf, bufLength);
	}
	CloseSerial();
	printf("Ending RunUDPListener \n");
	return 0;
}
