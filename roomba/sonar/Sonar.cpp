#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include "Sonar.h"

#define POLL_TIMEOUT 100
#define SONAR_MEASURE_RATE 100000

Sonar::Sonar(unsigned int gpioPinNum)
{
	_gpio = new Gpio(gpioPinNum);
	_minDist = 999;
	_maxDist = 0;
}

Sonar::~Sonar()
{
	delete _gpio;
}

struct timespec Sonar::TimeDiff()
{
	struct timespec temp;
	if ((_fallingTOI.tv_nsec - _risingTOI.tv_nsec) < 0)
	{
		temp.tv_sec = _fallingTOI.tv_sec - _risingTOI.tv_sec - 1;
		temp.tv_nsec = 1000000000 + _fallingTOI.tv_nsec - _risingTOI.tv_nsec;
	}
	else
	{
		temp.tv_sec = _fallingTOI.tv_sec - _risingTOI.tv_sec;
		temp.tv_nsec = _fallingTOI.tv_nsec - _risingTOI.tv_nsec;
	}
	return temp;
}

void Sonar::DisplayMeasurement()
{
	if (TimeDiff().tv_sec == 0)
	{
		float time = TimeDiff().tv_nsec/1000000000.f;
		float dist = 340.29*(time/2);
		if (dist < 3.f && dist > 0)
		{
			if (dist < _minDist)
			{
				_minDist = dist;
			}
			if (dist > _maxDist)
			{
				_maxDist = dist;
			}
			printf("Time taken is: %fs\n", time);
			printf("Dist is: %fm\t minDist: %fm\t maxDist: %fm\n", dist, _minDist, _maxDist);
		}
	}
}

void Sonar::StartPulse()
{
	_gpio->SetValue(LOW);
	usleep(10);
	_gpio->SetValue(HIGH);
	usleep(5);
	_gpio->SetValue(LOW);
}

int Sonar::Run()
{
	while(1)
	{
		StartPulse();
		_gpio->SetEdge(RISING);
		if (_gpio->Poll(POLL_TIMEOUT, _risingTOI) > 0)
		{
			_gpio->SetEdge(FALLING);
			while(_gpio->Poll(POLL_TIMEOUT, _fallingTOI) == -2);
			fflush(stdout);
		}
		DisplayMeasurement();
		usleep(SONAR_MEASURE_RATE);
	}
	return 0;
}
