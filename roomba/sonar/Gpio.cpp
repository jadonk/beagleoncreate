#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include "Gpio.h"

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64

Gpio::Gpio(unsigned int pinNum, bool isOut)
{
	_curVal = 2;		// Make it out of range to force write if out pin
	_fd = -1;				// Make it invalid to force initial open
	_pinNum = pinNum;

	Export();
	SetDir(isOut);	
}

Gpio::~Gpio()
{
	if( 0 <= _fd )
	{
		Unexport();
		CloseFd();
	}
}

int Gpio::Export()
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0)
	{
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", _pinNum);
	write(fd, buf, len);
	close(fd);
	return 0;
}

int Gpio::Unexport()
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0)
	{
		perror("gpio/unexport");
		return fd;
	}
	
	len = snprintf(buf, sizeof(buf), "%d", _pinNum);
	write(fd, buf, len);
	close(fd);
	return 0;
}

int Gpio::SetDir(bool out)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/direction", _pinNum);

	fd = open(buf, O_WRONLY);
	if (fd < 0)
	{
		perror("gpio/direction");
		return fd;
	}
	
	if( out )
		write(fd, "out", 4);
	else
		write(fd, "in", 3);

	close(fd);
	_isOut = out;
	return 0;
}

int Gpio::SetValue(GpioVal value)
{
	if( _isOut && _curVal != value )
	{
		if( _fd < 0 ) OpenFd();
		if( _fd < 0 ) return _fd;

		switch(value)
		{
			case HIGH:
				write(fd, "1", 2);
				break;
			case LOW:
				write(fd, "0", 2);
				break;
			default:
				break;
		}

		_curVal = value;
	}
	return 0;
}

int Gpio::GetValue(GpioVal & value)
{
	if( !_isOut )
	{
		if( _fd < 0 ) OpenFd();
		if( _fd < 0 ) return _fd;

		char ch;

		read(_fd, &ch, 1);

		if (ch != '0')
			value = HIGH;
		else
			value = LOW;

		_curVal = value;
	}
	return 0;
}

int Gpio::SetEdge(GpioEdge edge)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", _pinNum);

	fd = open(buf, O_WRONLY);
	if (fd < 0)
	{
		perror("gpio/setEdge");
		return fd;
	}

	switch(edge)
	{
		case RISING:
			write(fd, "rising", 7);
			break;
		case FALLING:
			write(fd, "falling", 8);
			break;
		case BOTH:
			write(fd, "both", 5);
			break;
		default:
			break;
	}
	
	close(fd);
	return 0;
}

void Gpio::OpenFd()
{
	int len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", _pinNum);
	
	if( _fd >= 0 ) close(_fd);
	
	if( _isOut )
		_fd = open(buf, O_RDWR);
	else
		_fd = open(buf, O_RDONLY);

	if ( _fd < 0 )
	{
		perror("gpio/fdOpen");
	}
}

void Gpio::CloseFd()
{
	close(_fd);
}

int Gpio::Poll(int usec, struct timespec & timeOfInterrupt)
{
	int len;
	char buf[MAX_BUF];
	struct pollfd fdset[1];
	
	memset((void*) fdset, 0, sizeof(fdset));
	fdset[0].fd = _fd;
	fdset[0].events = POLLPRI;

	int ret = poll(fdset, 1, usec);
	clock_gettime(CLOCK_REALTIME, &timeOfInterrupt);
	if (ret < 0)
	{
		perror("\npoll() failed!\n");
		return -1;
	}
	if (fdset[0].revents & POLLPRI) 
	{
		len = read(fdset[0].fd, buf, MAX_BUF);
		return ret;
	}
	if (fdset[0].revents & POLLERR)
	{
		printf("POLLERR\n");
		return -2;
	}
	// timeout
	return 0;
}
