#ifndef SONAR_H
#define SONAR_H

#include <time.h>
#include "Gpio.h"

#define SONAR_MEASURE_RATE 100000

class Sonar
{
public:
	Sonar(unsigned int gpioPinNum);
	~Sonar();
	
	float Run();
	bool isEnding;
	
private:
	Gpio * _gpio;
	float _minDist;
	float _maxDist;
	struct timespec _risingTOI;
	struct timespec _fallingTOI;
	
	struct timespec TimeDiff();
	float DisplayMeasurement();
	void StartPulse();
	
};

#endif
