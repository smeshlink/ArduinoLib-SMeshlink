#ifndef MXS2201_h
#define MXS2201_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"


class MXS2201  {
public:

	MXS2201(HardwareSerial mySerial);
	MXS2201();
	void StartSensor();
	void StopSensor();
	void get_info(float *latitude, float *longitude, float *altitude,float *speed_mps,int *year, byte *month, byte *day,
			byte *hour, byte *minute, byte *second,byte *hundredths,unsigned long *fix_age = 0);




private:

	static  HardwareSerial _mySerial;

};



#endif
