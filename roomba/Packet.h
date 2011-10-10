#ifndef PACKET_H
#define PACKET_H

#define MAXPACKETSIZE 1024
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
			float yaw;
			float timestamp;
		}data;

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
