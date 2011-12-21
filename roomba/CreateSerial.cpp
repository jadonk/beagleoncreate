#include "CreateSerial.h"

int CreateSerial::InitCreateSerial()
{
	debugMsg(__func__, "Entered");
	int fd;	// file description for the serial port

	fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);

	if(fd == -1) // if open is unsuccessful
	{
		debugMsg(__func__, "Unable to open /dev/ttyUSB0.");
		return -1;
	}
	else
	{
		fcntl(fd, F_SETFL, 0);
		debugMsg(__func__, "create serial port opened.");
	}

	// configure port
	struct termios portSettings;
	if (cfsetispeed(&portSettings, B57600) != 0)
		debugMsg(__func__, "Failed setting cfsetispeed");
	if (cfsetospeed(&portSettings, B57600) != 0)
		debugMsg(__func__, "Failed setting cfsetospeed");

	// set no parity, stop bits, databits
	portSettings.c_cflag &= ~PARENB;
	portSettings.c_cflag &= ~CSTOPB;
	portSettings.c_cflag &= ~CSIZE;
	portSettings.c_cflag |= CS8;

	if (tcsetattr(fd, TCSANOW, &portSettings) != 0)
		debugMsg(__func__, "Failed pushing portSettings");
	return (fd);
}

void CreateSerial::CloseCreateSerial(int fd)
{
	debugMsg(__func__, "Entered");
	close(fd);
}

void CreateSerial::SendSerialToCreate(int fd, char* buf, int bufLength)
{
	debugMsg(__func__, "Entered");
	write(fd, buf, bufLength);
	printf("bufLength: %d\n", bufLength);
	for (int i = 0; i < bufLength; i++)
	{
		printf("%i ", int(buf[i]));
	}
	printf("\n");
}

void* CreateSerial::CreateCallbackHandler(void* arg)
{
	int fd = *((int*) arg);
	char buf[MAXPACKETSIZE];
	int ret;
	int bufLength;
	int            max_fd;
	fd_set         input;
	struct timeval timeout;

	while(1)
	{
		if(isEnding)
			break;
			
		/* Initialize the input set */
		FD_ZERO(&input);
		FD_SET(fd, &input);
		max_fd = fd + 1;
		
		/* Initialize the timeout structure */
		timeout.tv_sec  = 10;
		timeout.tv_usec = 0;

		/* Do the select */
		ret = select(max_fd, &input, NULL, NULL, &timeout);

		/* See if there was an error */
		if (ret < 0)
		  debugMsg(__func__, "select failed");
		else if (ret != 0)
		{
			/* We have input */
			if (FD_ISSET(fd, &input))
			{
				bufLength = read(fd, buf, MAXPACKETSIZE);
				if (sendto(remoteSock, buf, bufLength, 0, (const struct sockaddr *) &remoteCreate, sizeof(struct sockaddr_in)) < 0) error("sendto");
				printf("%c", buf[0]);
			}
		}
		fflush(stdout);
	}
	return 0;
}

void* CreateSerial::CreateSerialHandler(void* arg)
{
	debugMsg(__func__, "Entered CreateSerialHandler");

	int sock, bufLength;
	socklen_t serverlen, fromlen;
	struct sockaddr_in server;
	struct sockaddr_in from;
	char buf[MAXPACKETSIZE];

	// initialize udp listener
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) error("Opening socket");
	serverlen = sizeof(server);
	bzero(&server, serverlen);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(CREATE_PORT);
	if (bind(sock, (struct sockaddr *)&server, serverlen) < 0) error("binding");
	fromlen = sizeof(struct sockaddr_in);

	// initialize Create serial communication
	int fd = InitCreateSerial();
	printf("fd created is %d\n", fd);
	pthread_t createCallbackThread;
	if (fd != -1)
	{
		printf("iRobot Create Callback Thread: %d.\n", 
			pthread_create(&createCallbackThread, NULL, CreateCallbackHandler, (void*)&fd));
	}
	debugMsg(__func__, "Ready to listen to Create message ...");
	while(1)
	{
		if (isEnding)
			break;

		bzero(&buf, sizeof(buf));
		bufLength = recvfrom(sock, buf, MAXPACKETSIZE, 
				0, (struct sockaddr *)&from, &fromlen);
		if (bufLength < 0) error("recvfrom");

		if (connectedHost != from.sin_addr.s_addr)
			continue;

		SendSerialToCreate(fd, buf, bufLength);
	}
	CloseCreateSerial(fd);

	debugMsg(__func__, "Ending CreateSerialHandler");
	return 0;
}