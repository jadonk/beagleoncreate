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
#include "Packet.h"
#include <termios.h>
#include <fcntl.h>
#include <poll.h>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>

#include "opencv/cv.h"
#include "opencv/highgui.h"

#include "ARtagLocalizer.h"

#define CREATE_PORT 8888
#define VIDEO_PORT 8855
#define ARTAG_PORT 8844

using namespace cv;
using namespace std;

bool showDebugMsg = true;
pthread_cond_t endCondition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t endMutex = PTHREAD_MUTEX_INITIALIZER;
bool isInit = false;
bool isEnding = false;
unsigned long connectedHost = 0;

static GMainLoop *loop       = NULL;
static GstElement *pipeline1 = NULL;
static GstElement *pipeline2 = NULL;
//AppSrc name
static const char app_src_name[]  = "app-src_01";
//AppSink name
static const char app_sink_name[] = "app-sink_01";
static int IMG_width = 320;
static int IMG_height = 240;
static IplImage * img;
static IplImage * gray;
static uchar * IMG_data;
static ARtagLocalizer ar;

int remoteSock;
struct sockaddr_in remoteVideo;
struct sockaddr_in remoteARtag;
struct sockaddr_in remoteCreate;

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void debugMsg(const char *func, const char *msg)
{
	if (showDebugMsg)	printf("[%s	] %s\n", func, msg);
}

void SendImage(IplImage * image)
{
	Packet packet;
	packet.type = DATA;
	packet.u.image.width = image->width;
	packet.u.image.height = image->height;
	memcpy(&packet.u.image.data, image->imageData, sizeof(packet.u.image.data));

	if (sendto(remoteSock, (unsigned char*)&packet, sizeof(packet), 0, (const struct sockaddr *)&remoteVideo, sizeof(struct sockaddr_in)) < 0) error("sendto");
}

void SendARtag()
{
	Packet packet;
	packet.type = DATA;
	int numARtags = ar.getARtagSize();
	for (int i = 0; i < numARtags; ++i)
	{
		ARtag* tag = ar.getARtag(i);
		packet.u.data.tagId = tag->getId();
		cv::Mat pose = tag->getPose();
		packet.u.data.x = pose.at<float>(0,3)/1000.f;
		packet.u.data.y = pose.at<float>(1,3)/1000.f;
		packet.u.data.z = pose.at<float>(2,3)/1000.f;
		packet.u.data.yaw = -atan2(pose.at<float>(1,0), pose.at<float>(0,0));
		if (packet.u.data.yaw < 0)
		{
			packet.u.data.yaw += 6.28;
		}
		if (sendto(remoteSock, (unsigned char*)&packet, sizeof(packet), 0, (const struct sockaddr *)&remoteARtag, sizeof(struct sockaddr_in)) < 0) error("sendto");
	}
}

GstFlowReturn new_buffer (GstAppSink *app_sink, gpointer user_data)
{
	GstBuffer *buffer = gst_app_sink_pull_buffer( (GstAppSink*) gst_bin_get_by_name( GST_BIN(pipeline1), app_sink_name));

	//processing...
	//handle imageData for processing
	IMG_data = (uchar*) img->imageData;
	// copies AppSink buffer data to the uchar vector of IplImage */
	memcpy(IMG_data, GST_BUFFER_DATA(buffer), GST_BUFFER_SIZE(buffer));
	// Image buffer is RGB, but OpenCV handles it as BGR, so channels R and B must be swapped */
	cvConvertImage(img,img,CV_CVTIMG_SWAP_RB);
	cvCvtColor(img,gray,CV_BGR2GRAY);

	//detect a image ...
	if(!ar.getARtagPose(gray, img, 0))
	{
//		debugMsg(__func__, "No artag in the view.");
	}
	SendARtag();
	IplImage* grayrz = cvCreateImage(cvSize(160,120), IPL_DEPTH_8U, 1);
	cvResize(gray, grayrz);
	SendImage(grayrz);
	cvReleaseImage(&grayrz);

	gst_object_unref(buffer);
	// Image buffer is RGB, but OpenCV handles it as BGR, so channels R and B must be swapped */
	/*cvConvertImage(img,img,CV_CVTIMG_SWAP_RB);
	//cvCvtColor(img,img,CV_RGB2BGR);
		
	memcpy(GST_BUFFER_DATA(buffer),IMG_data, GST_BUFFER_SIZE(buffer));
	//pushes the buffer to AppSrc, it takes the ownership of the buffer. you do not need to unref
	gst_app_src_push_buffer( GST_APP_SRC( gst_bin_get_by_name(GST_BIN(pipeline2),app_src_name)) , buffer);*/
	
	return GST_FLOW_OK;
}

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
        gchar *userdata = (gchar *) data;

        switch(GST_MESSAGE_TYPE(msg))
        {
                case GST_MESSAGE_EOS:
                {
			//sender check - pipeline 1 sends a EOS msg to AppSrc in pipeline2
			/*if (gascii_strcasecmp(userdata, gst_element_get_name(pipeline1)) == 0)
			 * {
			 * 	g_print("EOS detected (%s)\n", userdata);
			 * 	gst_app_src_end_of_stream( GST_APP_SRC( gst_bin_get_by_name( GST_BIN(pipeline2), app_src_name ) ) );
			 * }
			//sender check - when pipeline2 sends the EOS msg, quite.
			if ( g_ascii_strcasecmp(userdata, gst_element_get_name(pipeline2)) == 0)
			{
			        g_print("Finished playback (%s)\n",userdata);
				g_main_loop_quit(loop);
			}
			break;*/
		}

		case GST_MESSAGE_ERROR:
		{
			gchar *debug;
			GError *error;

			gst_message_parse_error(msg, &error, &debug);
			g_free(debug);

			g_printerr("Error in pipeline:%s\nError Message: %s\n", userdata, error->message);
			g_error_free(error);

			g_main_loop_quit(loop);
			break;
		}

		case GST_MESSAGE_STATE_CHANGED :
		{
			GstState oldstate;
			GstState newstate;
			GstState pending;

			gst_message_parse_state_changed (msg,&oldstate,&newstate,&pending);
			//g_debug("pipeline:%s old:%s new:%s pending:%s", userdata, gst_element_state_get_name(oldstate), gst_element_state_get_name(newstate), gst_element_state_get_name(pending));
			break;
		}

		case GST_MESSAGE_WARNING:
		{
			//gchar *debug;
			//GError *error;

			//gst_message_parse_warning (msg,&error,&debug);
			//g_warning("pipeline:%s",userdata);
			//g_warning("debug: %s", debug);
			//g_warning("error: %s", error->message);
			//g_free (debug);
			//g_error_free (error);
			break;
		}

		default:
			break;
	}
	return TRUE;
}

void* StreamARtagVideo(void* arg)
{
	// GStreamer stuff...
	GError *error = NULL;
	GstBus *bus = NULL;
	GstAppSinkCallbacks callbacks;
	gchar pipeline1_str[256];
	gchar pipeline2_str[256];

	// OpenCV stuff...
	int nChannels = 3;
	img = cvCreateImage( cvSize(IMG_width,IMG_height), IPL_DEPTH_8U, nChannels);
	gray = cvCreateImage( cvSize(IMG_width,IMG_height), IPL_DEPTH_8U, 1);

	// Initializing GStreamer
	//g_print("Initializing GStreamer.\n");
	gst_init(NULL, NULL);

	//g_print("Creating Main Loop.\n");
	loop = g_main_loop_new(NULL,FALSE);

	// Initializing ARtagLocalizer
	if (ar.initARtagPose(320, 240, 180.f) != 0)
	{
		debugMsg(__func__, "Failed to init ARtagLocalizer!");
		pthread_exit(NULL);
	}
	else
	{
		debugMsg(__func__, "ARtagLocalizer init successfully.");
	}

	//configuring pipeline parameters string
	// obs.: try g_strdup_printf
	int res = 0;
	res =  sprintf(pipeline1_str, "v4l2src ! ffmpegcolorspace ! video/x-raw-rgb, width=%d, height=%d ! appsink name=\"%s\"", IMG_width, IMG_height, app_sink_name);
	if (res < 0)
	{
		g_printerr("Error configuring pipeline1's string\n");
		pthread_exit(NULL);
	}

	res = sprintf(pipeline2_str, "appsrc name=\"%s\" ! queue ! ffmpegcolorspace ! video/x-raw-rgb, width=%d, height=%d ! ximagesink", app_src_name, IMG_width, IMG_height );
	if (res < 0)
	{
		g_printerr("Error configuring pipeline2's string \n");
		pthread_exit(NULL);
	}
	
	//debugging
	//g_print("%s\n",pipeline1_str);
	//creating pipeline1
	pipeline1 = gst_parse_launch(pipeline1_str, &error);

	if (error)
	{
		g_printerr("Error [%s]\n",error->message);
		pthread_exit(NULL);
	}

	//debugging
	//g_print("%s\n",pipeline2_str);
	//creating pipeline2
	pipeline2 = gst_parse_launch(pipeline2_str, &error);

	if (error)
	{
		g_printerr("Error [%s]\n",error->message);
		pthread_exit(NULL);
	}

	if (!gst_bin_get_by_name( GST_BIN(pipeline1), app_sink_name))
	{
		g_printerr("Error creating app-sink\n");
		pthread_exit(NULL);
	}

	if (!gst_bin_get_by_name( GST_BIN(pipeline2), app_src_name))
	{
		g_printerr("error creating app-src\n");
		pthread_exit(NULL);
	}

	// Adding msg handler to Pipeline1
	//g_print("Adding msg handler to %s\n", gst_element_get_name(pipeline1));
	bus = gst_pipeline_get_bus( GST_PIPELINE(pipeline1) );
	gst_bus_add_watch(bus, bus_call, gst_element_get_name(pipeline1));
	gst_object_unref(bus);

	// Adding msg handler to Pipeline1
	//g_print("Adding msg handler to %s\n",gst_element_get_name(pipeline2));
	bus = gst_pipeline_get_bus( GST_PIPELINE(pipeline2) );
	gst_bus_add_watch(bus, bus_call, gst_element_get_name(pipeline2));
	gst_object_unref(bus);

	//configuring AppSink's callback  (Pipeline1)
	callbacks.eos = NULL;
	callbacks.new_preroll = NULL;
	callbacks.new_buffer = new_buffer;
	gst_app_sink_set_callbacks( (GstAppSink*) gst_bin_get_by_name(GST_BIN(pipeline1), app_sink_name), &callbacks, NULL, NULL);

	//Set the pipeline to "playing" state
	//g_print("Setting pipeline1's state to \"playing\".\n");
	gst_element_set_state(pipeline1, GST_STATE_PLAYING);

	//Set the pipeline2 to "playing" state
	//g_print("Setting pipeline2's state to \"playing\".\n");
	gst_element_set_state(pipeline2, GST_STATE_PLAYING);

	// Iterate
	//g_print("Running...\n");
	debugMsg(__func__, "Running ARtag detection.");
	g_main_loop_run(loop);

	// Out of the main loop, clean up nicely
	//g_print("Stopping playback - pipeline1\n");
	gst_element_set_state(pipeline1, GST_STATE_NULL);

	//g_print("Stopping playback - pipeline2\n");
	gst_element_set_state(pipeline2, GST_STATE_NULL);

	//g_print("Deleting pipeline1.\n");
	gst_object_unref( GST_OBJECT(pipeline1) );

	//g_print("Deleting pipeline2.\n");
	gst_object_unref( GST_OBJECT(pipeline2) );

	//deleting image
	cvReleaseImage(&img);
	cvReleaseImage(&gray);

	//unref mainloop
	g_main_loop_unref(loop);

	pthread_exit(NULL);
}

void HandleControls(Packet & packet)
{
	//debugMsg(__func__, "Sending control is not yet implemented!");	
	if (packet.u.ctrl.data[0] == 1)
	{
		printf("pickup \n");
		system("bash pickup.sh");
	}
	else if (packet.u.ctrl.data[0] == 0)
	{
		printf("drop \n");
		system("bash drop.sh");
	}
	else if (packet.u.ctrl.data[0] == 2)
	{
		printf("resetarm \n");
		system("bash resetarm.sh");
	}

	printf("packet data: %d\n", packet.u.ctrl.data[0]);
}
void* StreamSensorData(void* arg)
{
	debugMsg(__func__, "Start streaming sensor data ...");
	unsigned long addr = *((unsigned long*) arg);
	printf("[%s	] addr: %d\n", __func__, (int)addr);
	isInit = true;
	pthread_t artagThread;
	printf("ARtag Thread: %d.\n", 
		pthread_create(&artagThread, NULL, StreamARtagVideo, NULL));
	while(1)
	{
		pthread_mutex_lock( &endMutex );
		if (isEnding)
		{
			isEnding = false;
			pthread_mutex_unlock( &endMutex );
			connectedHost = 0;
			g_main_loop_quit(loop);
			break;
		}
		pthread_mutex_unlock( &endMutex );
		sleep(5);
		// run artag
		// run sonar
		// run odometry
	}
	debugMsg(__func__, "End of streaming sensor data");
	isInit = false;
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

	connectedHost = packet.addr.s_addr;
	
	pthread_t createSerialThread;
	printf("iRobot Create Thread: %d.\n",
		pthread_create(&createSerialThread, NULL, CreateSerialHandler, NULL));

	pthread_t sensorThread;
	printf("Sensor Thread: %d.\n", 
		pthread_create(&sensorThread, NULL, StreamSensorData, (void*)&packet.addr.s_addr));
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
	server.sin_port = htons(atoi((char*)arg));
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
	if (argc < 2)
	{
		fprintf(stderr,"ERROR, no port provided\n");
		exit(0);
	}
	if (argc > 2)
	{
		showDebugMsg = (strcmp(argv[2],"hideDebug") == 0)?false:showDebugMsg;
	}


	pthread_t listenerThread;
	pthread_cond_init(&endCondition, NULL);
	printf("Listener Thread: %d.\n", 
		pthread_create(&listenerThread, NULL, ListenMessage, (void*)argv[1]));

	pthread_join(listenerThread, NULL);
	pthread_cond_destroy(&endCondition);
	// start sensor data thread
/*	StreamSensorData();
	// start message listening thread
	ListenMessage();*/

	// return to idle when receive a finish msg 

	// or restart when receive another init msg

	return 0;
}
