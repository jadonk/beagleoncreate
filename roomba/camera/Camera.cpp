#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <pthread.h>

#include "Packet.h"
#include "Camera.h"

using namespace cv;
GMainLoop* loop;
Camera* camera;

Camera::Camera(int remoteSock, struct sockaddr_in & videoPort, struct sockaddr_in & artagPort)
{
	_sock = remoteSock;
	_videoPort = videoPort;
	_artagPort = artagPort;
	camera = this;
}

Camera::~Camera()
{
}

void Camera::SendImage(IplImage * image)
{
	Packet packet;
	packet.type = DATA;
	packet.u.image.width = image->width;
	packet.u.image.height = image->height;
	memcpy(&packet.u.image.data, image->imageData, sizeof(packet.u.image.data));

	if (sendto(_sock, (unsigned char*)&packet, sizeof(packet), 0, (const struct sockaddr *)&_videoPort, sizeof(struct sockaddr_in)) < 0) printf("sendto\n");
}

void Camera::SendARtag()
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
		if (sendto(_sock, (unsigned char*)&packet, sizeof(packet), 0, (const struct sockaddr *)&_artagPort, sizeof(struct sockaddr_in)) < 0) printf("sendto\n");
	}
}

GstFlowReturn new_buffer (GstAppSink *app_sink, gpointer user_data)
{
	GstBuffer *buffer = gst_app_sink_pull_buffer( (GstAppSink*) gst_bin_get_by_name( GST_BIN(pipeline1), APPSINKNAME));

	//processing...
	//handle imageData for processing
	camera->IMG_data = (uchar*) camera->img->imageData;
	// copies AppSink buffer data to the uchar vector of IplImage */
	memcpy(camera->IMG_data, GST_BUFFER_DATA(buffer), GST_BUFFER_SIZE(buffer));
	// Image buffer is RGB, but OpenCV handles it as BGR, so channels R and B must be swapped */
	cvConvertImage(camera->img,camera->img,CV_CVTIMG_SWAP_RB);
	cvCvtColor(camera->img,camera->gray,CV_BGR2GRAY);

	//detect a image ...
	if(!ar.getARtagPose(camera->gray, camera->img, 0))
	{
//		printf("No artag in the view.\n");
	}
	SendARtag();
	IplImage* grayrz = cvCreateImage(cvSize(160,120), IPL_DEPTH_8U, 1);
	cvResize(camera->gray, grayrz);
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

int Camera::StreamARtagVideo()
{
	// GStreamer stuff...
	GError *error = NULL;
	GstBus *bus = NULL;
	GstAppSinkCallbacks callbacks;
	gchar pipeline1_str[256];
	gchar pipeline2_str[256];

	// OpenCV stuff...
	int nChannels = 3;
	img = cvCreateImage( cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, nChannels);
	gray = cvCreateImage( cvSize(IMG_WIDTH,IMG_HEIGHT), IPL_DEPTH_8U, 1);

	// Initializing GStreamer
	//g_print("Initializing GStreamer.\n");
	gst_init(NULL, NULL);

	//g_print("Creating Main Loop.\n");
	loop = g_main_loop_new(NULL,FALSE);

	// Initializing ARtagLocalizer
	if (ar.initARtagPose(320, 240, 180.f) != 0)
	{
		printf("Failed to init ARtagLocalizer!\n");
		pthread_exit(NULL);
	}
	else
	{
		printf("ARtagLocalizer init successfully.\n");
	}

	//configuring pipeline parameters string
	// obs.: try g_strdup_printf
	int res = 0;
	res =  sprintf(pipeline1_str, "v4l2src ! ffmpegcolorspace ! video/x-raw-rgb, width=%d, height=%d ! appsink name=\"%s\"", IMG_WIDTH, IMG_HEIGHT, APPSINKNAME);
	if (res < 0)
	{
		g_printerr("Error configuring pipeline1's string\n");
		pthread_exit(NULL);
	}

	res = sprintf(pipeline2_str, "appsrc name=\"%s\" ! queue ! ffmpegcolorspace ! video/x-raw-rgb, width=%d, height=%d ! ximagesink", APPSRCNAME, IMG_WIDTH, IMG_HEIGHT );
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

	if (!gst_bin_get_by_name( GST_BIN(pipeline1), APPSINKNAME))
	{
		g_printerr("Error creating app-sink\n");
		pthread_exit(NULL);
	}

	if (!gst_bin_get_by_name( GST_BIN(pipeline2), APPSRCNAME))
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
	gst_app_sink_set_callbacks( (GstAppSink*) gst_bin_get_by_name(GST_BIN(pipeline1), APPSINKNAME), &callbacks, NULL, NULL);

	//Set the pipeline to "playing" state
	//g_print("Setting pipeline1's state to \"playing\".\n");
	gst_element_set_state(pipeline1, GST_STATE_PLAYING);

	//Set the pipeline2 to "playing" state
	//g_print("Setting pipeline2's state to \"playing\".\n");
	gst_element_set_state(pipeline2, GST_STATE_PLAYING);

	// Iterate
	//g_print("Running...\n");
	printf("Running ARtag detection.\n");
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
	return 0;
}

void Camera::QuitMainLoop()
{
	g_main_loop_quit(loop);
}
