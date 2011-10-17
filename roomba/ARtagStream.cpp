#include "ARtagStream.h"
#include <stdio.h>

static GMainLoop *loop       = NULL;
static GstElement *pipeline1 = NULL;
static GstElement *pipeline2 = NULL;

//AppSrc name
static const char app_src_name[]  = "app-src_01";

//AppSink name
static const char app_sink_name[] = "app-sink_01";

static int IMG_width = 320;
static int IMG_height = 240;

GstFlowReturn new_buffer (GstAppSink *app_sink, gpointer user_data);
ARtagStream::ARtagStream(pthread_cond_t *cond, pthread_mutex_t *mutex)
	: imgReadyCond(cond), imgReadyMutex(mutex)
{
}

ARtagStream::~ARtagStream()
{

}

unsigned char* ARtagStream::getData()
{
	return IMG_data;
}

int ARtagStream::Run()
{
	// GStreamer stuff...
	GError *error = NULL;
	GstAppSinkCallbacks callbacks;
	gchar pipeline1_str[256];
	gchar pipeline2_str[256];
	gst_init(NULL, NULL);
	loop = g_main_loop_new(NULL,FALSE);
	
	//configuring pipeline parameters string
	// obs.: try g_strdup_printf
	int res = 0;
	res =  sprintf(pipeline1_str, "v4l2src ! ffmpegcolorspace ! video/x-raw-rgb, width=%d, height=%d ! appsink name=\"%s\"", IMG_width, IMG_height, app_sink_name);
	if (res < 0)
	{
		g_printerr("Error configuring pipeline1's string\n");
		return -1;
	}
	
	//debugging
	g_print("%s\n",pipeline1_str);
	//creating pipeline1
	pipeline1 = gst_parse_launch(pipeline1_str, &error);
	if (error)
	{
		g_printerr("Error [%s]\n",error->message);
		return -1;
	}
	if (!gst_bin_get_by_name( GST_BIN(pipeline1), app_sink_name))
	{
		g_printerr("Error creating app-sink\n");
		return -1;
	}
	
	//configuring AppSink's callback  (Pipeline1)
	callbacks.eos = NULL;
	callbacks.new_preroll = NULL;
	callbacks.new_buffer = new_buffer;
	gst_app_sink_set_callbacks( (GstAppSink*) gst_bin_get_by_name(GST_BIN(pipeline1), app_sink_name), &callbacks, NULL, NULL);
	
	//Set the pipeline to "playing" state
	g_print("Setting pipeline1's state to \"playing\".\n");
	gst_element_set_state(pipeline1, GST_STATE_PLAYING);
	
	// Iterate
	g_print("Running...\n");
	g_main_loop_run(loop);
	
	// Out of the main loop, clean up nicely
	g_print("Stopping playback - pipeline1\n");
	gst_element_set_state(pipeline1, GST_STATE_NULL);
	g_print("Deleting pipeline1.\n");
	gst_object_unref( GST_OBJECT(pipeline1) );
	//unref mainloop
	g_main_loop_unref(loop);
	
	return 0;
}

GstFlowReturn new_buffer (GstAppSink *app_sink, gpointer user_data)
{
	GstBuffer *buffer = gst_app_sink_pull_buffer( (GstAppSink*) gst_bin_get_by_name( GST_BIN(pipeline1), app_sink_name));
	pthread_mutex_lock(this.imgReadyMutex);
	// copies AppSink buffer data to the uchar vector of IplImage */
	memcpy(IMG_data, GST_BUFFER_DATA(buffer), GST_BUFFER_SIZE(buffer));
	pthread_mutex_unlock(this.imgReadyMutex);
	pthread_cond_signal(this.imgReadyCond);

	return GST_FLOW_OK;
}
