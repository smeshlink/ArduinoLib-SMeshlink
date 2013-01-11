#ifndef MXS1301_h
#define MXS1301_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"


class MXS1301  {
public:


	MXS1301();
	void StartSensor();
	void StopSensor();
	uint16_t getCO2();


private:



};



#endif
