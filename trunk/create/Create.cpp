#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <termios.h>
#include <fcntl.h>
#include <poll.h>

#include "../Packet.h"
#include "Create.h"

/*! \file Create.cpp */

/*! The serial port path in the sys_fs. */
#define CREATE_SERIAL_PORT "/dev/ttyUSB0"
/*! The serial port baudrate for serial communication with iRobot Create. */
#define CREATE_SERIAL_BRATE B57600

/*!
 * 	\class Create Create.h "Create.h"
 *	\brief This class handles all serial communication with iRobot Create.
 */

/*! \fn Create::Create(int sock, struct sockaddr_in & createPort, unsigned long connectedHost)
 *  \brief A constructor for Create class. The serial communication is initialized here.
 *  \param sock The remote socket that was initialized and ready to be used.
 * 	\param createPort The port for sending and receiving stuff for Create.
 * 	\param connectedHost The connected host's ip address to avoid muliple connection.
 */
Create::Create(unsigned long connectedHost)
{
	_fd = -1;
	_connectedHost = connectedHost;
	isEnding = false;
	pthread_mutex_init(&_serialMutex, NULL);
	InitSerial();
}

/*! \fn Create::~Create()
 * 	\brief A destructor for Create class. The serial communication is cleaned up here.
 */
Create::~Create()
{
	pthread_mutex_destroy(&_serialMutex);
	CloseSerial();
}

/*! \fn int Create::InitSerial()
 * 	\brief To initialize the serial communication.
 * 	\return fd, the file descriptor on success, -1 on fail.
 */
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

/*! \fn void Create::CloseSerial()
 * 	\brief Close the serial communication.
 */
void Create::CloseSerial()
{
	close(_fd);
}

/*! \fn void Create::SendSerial(char* buf, int bufLength)
 * 	\brief Send the stuff over to serial interface to iRobot Create
 * 	\param buf The "stuff" to be sent over.
 * 	\param bufLength The size of the stuff.
 */
void Create::SendSerial(char* buf, int bufLength)
{
	if (_fd == -1)
	{
		printf("ERROR: _fd is not initialized\n");
		return;
	}

	if (bufLength == 0)
		return;

	pthread_mutex_lock(&_serialMutex);
	int ret = write(_fd, buf, bufLength);
	pthread_mutex_unlock(&_serialMutex);
	if (ret == -1)
	{
		printf("ERROR: write error occured.\n");
		return;
	}
	printf("Sending to Create: \n\t\t");
	for (int i = 0; i < bufLength; i++)
	{
		printf("%i ", int(buf[i]));
	}
	printf("\n");
}

/*! \fn int Create::RunSerialListener()
 *  \brief The serial listener to listen anything coming from the iRobot Create.
 * 	\return 0 on success, -1 on fail.
 */
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
				pthread_mutex_lock(&_serialMutex);
				bufLength = read(_fd, buf, MAXPACKETSIZE);
				pthread_mutex_unlock(&_serialMutex);

				if (send(_sock, buf, bufLength, 0) != bufLength)
					printf("ERROR: send\n");
#if 0
				if (sendto(_sock, buf, bufLength, 0, (const struct sockaddr *) &_createPort, sizeof(struct sockaddr_in)) < 0) printf("ERROR: sendto\n");
#endif
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

/*! \fn int Create::RunTCPListener(int & sock)
 * 	\brief The main loop of the TCP listener.
 * 	\return 0 on success, -1 on fail.
 */
int Create::RunTCPListener()
{
	int sock, clientsock, bufLength;
	socklen_t serverlen, fromlen;
	struct sockaddr_in server;
	struct sockaddr_in from;
	char buf[MAXPACKETSIZE];

	// initialize tcp listener
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) error("Opening socket");
	serverlen = sizeof(server);
	bzero(&server, serverlen);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(CREATE_PORT);
	if (bind(sock, (struct sockaddr *)&server, serverlen) < 0) error("binding");
	fromlen = sizeof(struct sockaddr_in);

	printf("Ready to listen to Create message ...\n");
	if (listen(sock, 1) < 0) 
	{
		printf("failed to listen on Create socket\n");
		return -1;
	}
	printf("end of listen TCP socket %d\n", sock);
	if ((clientsock = accept(sock, (struct sockaddr *) &from, &fromlen)) < 0)
	{
		printf("failed to accept client connection on Create socket\n");
	       return -1;	
	}
	printf("accepted the Create TCP connection\n");
	_sock = clientsock;
	while(1)
	{
		if (isEnding)
		{
			printf("RunUDPListener received Ending flag\n");
			break;
		}
		
		bzero(&buf, sizeof(buf));
#if 0
		bufLength = timeout_recvfrom(_sock, buf, MAXPACKETSIZE, 
				(struct sockaddr *) &from, &fromlen, 1);
#endif
		#if 1
		bufLength = recvfrom(clientsock, buf, MAXPACKETSIZE, 
				0, (struct sockaddr *)&from, &fromlen);
		#endif

		if (bufLength == 0) continue;
		if (bufLength < 0) printf("ERROR: recvfrom\n");
	
		if (_connectedHost != from.sin_addr.s_addr)
			continue;

		SendSerial(buf, bufLength);
	}
	CloseSerial();
	printf("Ending RunUDPListener \n");
	return 0;
}
