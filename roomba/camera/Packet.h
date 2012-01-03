#ifndef PACKET_H
#define PACKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAXPACKETSIZE 5000 
enum PacketType
{
	INIT = 1,
	END,
	CTRL,
	DATA,
	ERROR,
	SHUTDOWN,
	UNKNOWN
};

struct Packet
{
	PacketType type;
	unsigned short port;
	struct in_addr addr;
	union
	{
		struct
		{
			char data[256];
		}init;

		struct
		{
			char data[256];
		}end;

		struct
		{
			char data[256];
		}ctrl;

		struct
		{
			int tagId;
			float x;
			float y;
			float z;
			float yaw;
		}data;

		struct
		{
			int width;
			int height;
			char data[19200];
		}image;

		struct
		{
			int errorCode;
			char data[256];
		}error;

		struct
		{
			char data[256];
		}shutdown;
		struct
		{
			char data[256];
		}unknown;
	}u;
};

#endif
