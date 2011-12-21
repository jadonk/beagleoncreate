#include "CreateSerialReceiver.h"
#include "Packet.h"

CreateSerialReceiver::CreateSerialReceiver(int fd, int sock, struct sockaddr_in * remote)
{
	_fd = fd;
	_sock = sock;
	_remote = remote;
	isEnding = false;
}

void CreateSerialReceiver::Run()
{
	char buf[MAXPACKETSIZE];
	int ret;
	int bufLength;
	int max_fd;
	fd_set input;
	struct timeval timeout;

	while(1)
	{
		if (isEnding)
			break;

		/* Initialize the input set */
		FD_ZERO(&input);
		FD_SET(_fd, &input);
		max_fd = _fd + 1;

		/* Initialize the timeout structure */
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;

		/* Do the Select */
		ret = select(max_fd, &input, NULL, NULL, &timeout);

		/* See if there was an error */
		if (ret < 0)
			debugMsg(__func__, "select failed");
		else if (ret != 0)
		{
			/* We have input */
			if (FD_ISSET(_fd, &input))
			{
				bufLength = read(_fd, buf, MAXPACKETSIZE);
				if (sendto(_sock, buf, bufLength, 0, (const struct sockaddr *) _remote, sizeof(struct sockaddr_in)) < 0) error("sendto");
				printf("%c", buf[0]);
			}
		}
		fflush(stdout);
	}
}
