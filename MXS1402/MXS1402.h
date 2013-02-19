#ifndef MXS1401_h
#define MXS1401_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"


class MXS1402  {
public:


	MXS1402();
	void StartSensor();
	void StopSensor();
	int getSoilHumdity();
	float getSoilTemp();


private:



};



#endif
