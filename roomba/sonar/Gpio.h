#ifndef GPIO_H
#define GPIO_H

#include <time.h>
/*! \file Gpio.h
 */

/*! GpioVal enum for gpio value. */
enum GpioVal
{ 
	HIGH, 		/*! gpio value is high. */
	LOW,		/*! gpio value is low. */
	UNKNOWNVAL	/*! gpio value is in unknown state?! */
};

/*! GpioEdge enum for gpio input trigger edge. */
enum GpioEdge
{
	RISING,		/*! gpio input trigger on rising edge. */
	FALLING,	/*! gpio input trigger on falling edge. */
	BOTH		/*! gpio input trigger on both edge. */
};

class Gpio
{
public:
	Gpio(unsigned int pinNum, bool isOut = true);
	~Gpio();
	
	int GetValue(GpioVal & value);
	
	int SetEdge(GpioEdge edge);
	int SetValue(GpioVal value);
	int SetDir(bool out); 
	
	int Poll(int usec, struct timespec & timeOfInterrupt);
	
private:
	bool _isOut;
	int _fd;
	GpioVal _curVal;
	unsigned int _pinNum;
	
	int Export();
	int Unexport();
	void OpenFd();
	void CloseFd();
	
};

#endif
