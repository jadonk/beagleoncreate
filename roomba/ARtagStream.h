#ifndef ARTAGSTREAM_H
#define ARTAGSTREAM_H

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>

#include <unistd.h>
#include <iostream>

#include <pthread.h>
using namespace std;

class ARtagStream
{
	pthread_cond_t *imgReadyCond;
	pthread_mutex_t *imgReadyMutex;
	unsigned char * IMG_data;
			
	int Run(void);
	
public:
	ARtagStream(pthread_cond_t * cond, pthread_mutex_t * mutex);
	~ARtagStream(void);
	
	unsigned char* getData(void);
};
#endif