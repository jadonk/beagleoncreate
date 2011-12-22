#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/resource.h>

#include "Sonar.h"

int main(int argc, char **argv, char **envp)
{
	unsigned int gpioPinNum;
	setpriority(PRIO_PROCESS, 0, -20);

	if (argc < 2)
	{
		printf("Usage: sonar <gpioPIN#>\n\n");
		printf("Read one sonar sensor data in the GPIO pin\n");
		exit(-1);
	}

	try
	{
		gpioPinNum = atoi(argv[1]);
		Sonar * sonar = new Sonar(gpioPinNum);
		sonar->Run();
	}
	catch( ... )
	{
		printf("Some exception occured!\n");
	}
	return 0;
}