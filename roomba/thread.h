#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>

class PThread
{
	pthread_t thread_id;

	public:

	PThread()
	{
		pthread_create( &thread_id, NULL, EntryPoint, (void*) this);
	}

	static void *EntryPoint(void *thread)
	{
		PThread *self = static_cast<PThread *>(thread);
		self->Run();
		return 0;
	}

	virtual void Run() = 0;
};

#endif

