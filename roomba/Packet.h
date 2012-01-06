#ifndef PACKET_H
#define PACKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAXPACKETSIZE 5000 
#define MAXARTAGSEEN 10
enum PacketType
{
	INIT = 1,
	END,
	CTRL,
	DATA,
	IMAGE,
	SONAR,
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
			int tagId[MAXARTAGSEEN];
			float x[MAXARTAGSEEN];
			float y[MAXARTAGSEEN];
			float z[MAXARTAGSEEN];
			float yaw[MAXARTAGSEEN];
		}data;

		struct
		{
			int width;
			int height;
			char data[19200];
		}image;
		
		struct
		{
			float dist1;
			float dist2;
			float dist3;
		}sonar;

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


