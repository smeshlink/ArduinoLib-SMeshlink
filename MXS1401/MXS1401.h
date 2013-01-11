#ifndef MXS1401_h
#define MXS1401_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"


class MXS1401  {
public:


	MXS1401();
	void StartSensor();
	void StopSensor();
	int getSoilHumdity();


private:



};



#endif
