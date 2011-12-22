#ifndef SONAR_H
#define SONAR_H

#include <time.h>

class Sonar
{
public:
	Sonar(unsigned int gpioPinNum);
	~Sonar();
	
	int Run();
	
	
private:
	Gpio * _gpio;
	float _minDist;
	float _maxDist;
	struct timespec _risingTOI;
	struct timespec _fallingTOI;
	
	void TimeDiff();
	void DisplayMeasurement();
	void StartPulse();
	
}

#endif