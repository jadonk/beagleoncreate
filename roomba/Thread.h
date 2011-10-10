#ifndef THREAD_H
#define THREAD_H

#include "pthread.h"

class Thread
{
public:
	Thread();
	int Start(void *arg);
	int Run(void *arg);
	static void * EntryPoint(void*);
	virtual void Setup();
	virtual void Execute(void*);
	void * Arg() const {return _Arg;}
	void Arg(void *a) {_Arg = a;}
private:
	pthread_t _ThreadId;
	void * _Arg;
};

#endif
