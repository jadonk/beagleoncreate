#include "Thread.h"

Thread::Thread()
{
}

int Thread::Start(void *arg)
{
	Arg(arg); // store user data;
	return pthread_create(&_ThreadId, NULL, EntryPoint, this);
}

int Thread::Run(void *arg)
{
	Setup();
	Execute(arg);
	return 0;
}

/* static */
void * Thread::EntryPoint(void * pthis)
{
	Thread * pt = (Thread*) pthis;
	pt->Run(pt->Arg());
}

// virtual
void Thread::Setup()
{
	// Do any setup here
}

// virtual
void Thread::Execute(void *arg)
{
	// Your code goes here
}
