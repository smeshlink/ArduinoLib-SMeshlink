#ifndef MXS1101_h
#define MXS1101_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"

class MXS1101  {
public:

	MXS1101();
	void StartSensor();
	void StopSensor();
	float getTempC();

private:


};



#endif
