#ifndef MXS1501_h
#define MXS1501_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"


class MXS1501  {
public:

	MXS1501();
	void StartSensor();
	void StopSensor();
	float getLight();


private:



};



#endif
