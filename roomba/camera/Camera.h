#ifndef CAMERA_H
#define CAMERA_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>

#include "opencv/cv.h"
#include "opencv/highgui.h"

#include "ARtagLocalizer.h"

#define APPSRCNAME "app-src_01"
#define APPSINKNAME "app-sink_01"
#define IMG_WIDTH 320
#define IMG_HEIGHT 240

class Camera
{
public:
	Camera(int remoteSock, struct sockaddr_in & videoPort, struct sockaddr_in & artagPort);
	~Camera();
	
	void StreamARtagVideo();
	void QuitMainLoop();
	
private:
	void SendImage(IplImage * image);
	void SendARtag();
	GstFlowReturn new_buffer (GstAppSink *app_sink, gpointer user_data);
	static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data);
	
	int _sock;
	struct sockaddr_in _videoPort;
	struct sockaddr_in _artagPort;
	
	GMainLoop *loop;
	GstElement *pipeline1;
	GstElement *pipeline2;
	IplImage * img;
	IplImage * gray;
	uchar * IMG_data;
	ARtagLocalizer ar;
};

#endif
