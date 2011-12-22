#ifndef GPIO_H
#define GPIO_H

#include <time.h>

enum GpioVal
{ 
	HIGH, 
	LOW
};

enum GpioEdge
{
	RISING,
	FALLING,
	BOTH
};

class Gpio
{
public:
	Gpio(unsigned int pinNum, bool isOut = true);
	~Gpio();
	
	int GetValue(GpioVal & value);
	
	int SetEdge(GpioEdge edge);
	int SetValue(GpioVal value);
	
	int Poll(int usec, struct timespec & timeOfInterrupt);
	
private:
	bool _isOut;
	int _fd;
	unsigned int _curVal;
	unsigned int _pinNum;
	
	int Export();
	int Unexport();
	void OpenFd();
	void CloseFd();
	int SetDir(bool out); 
	
};

#endif