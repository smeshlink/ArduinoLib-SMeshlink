#ifndef MXS1201_h
#define MXS1201_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"

class MXS1201  {
public:

	MXS1201();
	void StartSensor();
	void StopSensor();
	float getTempC();
	float getHumidity();

private:


};



#endif
