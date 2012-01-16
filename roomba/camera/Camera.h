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

/*! \file Camera.h
 */

/*!
  Set the app source name for gstreamer.
*/
#define APPSRCNAME "app-src_01"

/*!
  Set the app sink name for gstreamer.
*/
#define APPSINKNAME "app-sink_01"

/*!
  Set the image width used for gstreamer.
*/
#define IMG_WIDTH 320

/*!
  Set the image height used for gstreamer.
*/
#define IMG_HEIGHT 240

class Camera
{
public:
	Camera(int remoteSock, struct sockaddr_in & videoPort, struct sockaddr_in & artagPort);
	~Camera();
	
	int StreamARtagVideo();
	void QuitMainLoop();
	void SendImage(IplImage * image);
	void SendARtag();

	GMainLoop *loop;	
	GstElement *pipeline1;
	GstElement *pipeline2;
	IplImage * img;
	IplImage * gray;
	uchar * IMG_data;
	ARtagLocalizer * ar;
	
private:
	int Setup();
	void CleanUp();

	int _sock;
	struct sockaddr_in _videoPort;
	struct sockaddr_in _artagPort;
};

#endif
