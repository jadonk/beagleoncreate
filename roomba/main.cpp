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
#include <sys/resource.h>

#include "Packet.h"
#include "control/Control.h"
#include "camera/Camera.h"
#include "create/Create.h"
#include "sonar/Sonar.h"

#define CREATE_PORT 8888
#define BEAGLE_PORT 8866
#define VIDEO_PORT 8855
#define ARTAG_PORT 8844
#define SONAR_PORT 8833

#define SONAR_GPIO1 137
#define SONAR_GPIO2 136
#define SONAR_GPIO3 135

using namespace std;

// main variables
bool showDebugMsg = true;
pthread_cond_t endCondition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t endMutex = PTHREAD_MUTEX_INITIALIZER;
bool isInit = false;
bool isEnding = false;
unsigned long connectedHost = 0;
int remoteSock;
int createUDPsock = -1;
struct sockaddr_in remoteVideo;
struct sockaddr_in remoteARtag;
struct sockaddr_in remoteCreate;
struct sockaddr_in remoteSonar;

// class variables
Camera * camera;
Create * create;
Sonar * sonar1;
Sonar * sonar2;
Sonar * sonar3;

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void debugMsg(const char *func, const char *msg)
{
	if (showDebugMsg)	printf("[%s	] %s\n", func, msg);
}

void HandleControls(Packet & packet)
{
	//debugMsg(__func__, "Sending control is not yet implemented!");	
	if (packet.u.ctrl.data[0] == 1)
	{
		printf("pickup \n");
		system(PICKUPBASH);
	}
	else if (packet.u.ctrl.data[0] == 0)
	{
		printf("drop \n");
		system(DROPBASH);
	}
	else if (packet.u.ctrl.data[0] == 2)
	{
		printf("resetarm \n");
		system(RESETBASH);
	}

	printf("packet data: %d\n", packet.u.ctrl.data[0]);
}

void* RunARtagVideo(void* arg)
{
	camera->StreamARtagVideo();
	pthread_exit(NULL);
}

void* CreateSerialListener(void* arg)
{
	create->RunSerialListener();
	pthread_exit(NULL);
}

void* CreateUDPListener(void* arg)
{
	create->RunUDPListener(createUDPsock);
	pthread_exit(NULL);
}

void* SonarSender(void* arg)
{
	setpriority(PRIO_PROCESS, 0, -20);
	float dist1 = -1.f;
	float dist2 = -1.f;
	float dist3 = -1.f;
	Packet packet;
	packet.type = SONAR;
	
	while(1)
	{
		if (sonar1->isEnding)
			break;
		
		dist1 = sonar1->Run();	
		usleep(SONAR_WAIT_TIME);
		dist2 = sonar2->Run();	
		usleep(SONAR_WAIT_TIME);
		dist3 = sonar3->Run();			
		usleep(SONAR_WAIT_TIME);
		if (dist1 != -1.f )//&& dist2 != -1.f && dist3 != -1.f)
		{
			// send out dist
			packet.u.sonar.dist1 = dist1;
			packet.u.sonar.dist2 = dist2;
			packet.u.sonar.dist3 = dist3;
			if (sendto(remoteSock, (unsigned char*)&packet, sizeof(packet), 0, (const struct sockaddr *)&remoteSonar, sizeof(struct sockaddr_in)) < 0) printf("sendto\n");
		}
		usleep(SONAR_MEASURE_RATE);
	}
	pthread_exit(NULL);
}

void* Dumbload(void* arg)
{
	while(1)
	{
	}
	pthread_exit(NULL);
}

void* StreamSensorData(void* arg)
{
	debugMsg(__func__, "Start streaming sensor data ...");
	isInit = true;
	
	camera = new Camera(remoteSock, remoteVideo, remoteARtag);
	create = new Create(remoteSock, remoteCreate, connectedHost);
	sonar1 = new Sonar(SONAR_GPIO1);
	sonar2 = new Sonar(SONAR_GPIO2);
	sonar3 = new Sonar(SONAR_GPIO3);

	// run camera
	pthread_t cameraThread;
	printf("Camera Thread: %d.\n", 
		pthread_create(&cameraThread, NULL, RunARtagVideo, NULL));
	
	// run create control	
	pthread_t createSerialThread;
	printf("iRobot Create SerialListner Thread: %d.\n", 
		pthread_create(&createSerialThread, NULL, CreateSerialListener, NULL));

	pthread_t createUDPThread;
	printf("iRobot Create UDPListner Thread: %d.\n", 
		pthread_create(&createUDPThread, NULL, CreateUDPListener, NULL));
		
	// run sonar
	pthread_t sonarThread;
	printf("Sonar Thread: %d.\n", 
		pthread_create(&sonarThread, NULL, SonarSender, NULL));

	// run dumbload
	/*pthread_t dumbloadThread;
	printf("dumbload Thread: %d.\n", 
		pthread_create(&dumbloadThread, NULL, Dumbload, NULL));*/
	
	while(1)
	{
		pthread_mutex_lock( &endMutex );
		if (isEnding)
		{
			isEnding = false;
			pthread_mutex_unlock( &endMutex );
			connectedHost = 0;
			camera->QuitMainLoop();
			create->isEnding = true;
			sonar1->isEnding = true;
			break;
		}
		pthread_mutex_unlock( &endMutex );
		sleep(5);
	}
	debugMsg(__func__, "Waiting for threads halt...");
	isInit = false;
	
	pthread_join(cameraThread, NULL);
	debugMsg(__func__, "cameraThread halted");
	pthread_join(createSerialThread, NULL);
	debugMsg(__func__, "createSerialThread halted");
	pthread_join(createUDPThread, NULL);
	debugMsg(__func__, "createUDPThread halted");
	pthread_join(sonarThread, NULL);
	debugMsg(__func__, "sonarThread halted");
	delete camera;
	delete create;
	delete sonar1;
	delete sonar2;
	delete sonar3;
	debugMsg(__func__, "End of streaming sensor data.");
	pthread_exit(NULL);
}

void MakeConnection(Packet & packet)
{
	if (isInit)
	{
		debugMsg(__func__, "Connection is already occupied.");
		return;
	}
	remoteSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (remoteSock < 0) error("socket");

	remoteVideo.sin_family = AF_INET;
	remoteVideo.sin_addr.s_addr = packet.addr.s_addr;
	remoteVideo.sin_port = htons(VIDEO_PORT);

	remoteARtag.sin_family = AF_INET;
	remoteARtag.sin_addr.s_addr = packet.addr.s_addr;
	remoteARtag.sin_port = htons(ARTAG_PORT);

	remoteCreate.sin_family = AF_INET;
	remoteCreate.sin_addr.s_addr = packet.addr.s_addr;
	remoteCreate.sin_port = htons(CREATE_PORT);

	remoteSonar.sin_family = AF_INET;
	remoteSonar.sin_addr.s_addr = packet.addr.s_addr;
	remoteSonar.sin_port = htons(SONAR_PORT);

	connectedHost = packet.addr.s_addr;

	pthread_t sensorThread;
	printf("Sensor Thread: %d.\n", 
		pthread_create(&sensorThread, NULL, StreamSensorData, NULL));
}

void ProcessPackets(Packet & packet)
{
	switch(packet.type)
	{
		case INIT:
			debugMsg(__func__, "======= packet received, type: INIT");
			MakeConnection(packet);
			break;
		case END:
			debugMsg(__func__, "======= packet received, type: END");
			if (!isInit)
				debugMsg(__func__, "No connection was initialized.");
			else
			{
				pthread_mutex_lock( &endMutex );
				isEnding = true;
				pthread_mutex_unlock( &endMutex );
			}
			break;
		case CTRL:
			debugMsg(__func__, "======= packet received, type: CTRL");
			if (connectedHost == packet.addr.s_addr)
			{
				HandleControls(packet);
			}
			else
			{
				debugMsg(__func__, "There is no connection made with this client, please INIT first");
			}
			break;
		case DATA:
			debugMsg(__func__, "======= packet received, type: DATA");
			break;
		case ERROR:
			debugMsg(__func__, "======= packet received, type: ERROR");
			break;
		case SHUTDOWN:
			debugMsg(__func__, "======= packet received, type: SHUTDOWN");
			pthread_mutex_lock( &endMutex );
			isEnding = true;
			pthread_mutex_unlock( &endMutex );
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
	server.sin_port = htons(BEAGLE_PORT);
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

		memcpy((unsigned char*)&packet, buf, 256);
		packet.addr = from.sin_addr;
		packet.port = from.sin_port;
		ProcessPackets(packet);
		if (packet.type == END) debugMsg(__func__, "Waiting for INIT message ...");
		if (packet.type == SHUTDOWN) break;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	// loading input parameters
	if (argc > 1)
	{
		showDebugMsg = (strcmp(argv[2],"hideDebug") == 0)?false:showDebugMsg;
	}

	pthread_t listenerThread;
	pthread_cond_init(&endCondition, NULL);
	printf("Listener Thread: %d.\n", 
		pthread_create(&listenerThread, NULL, ListenMessage, (void*)argv[1]));

	pthread_join(listenerThread, NULL);
	pthread_cond_destroy(&endCondition);

	return 0;
}
