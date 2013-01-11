#ifndef MXS1202_h
#define MXS1202_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"

class MXS1202  {
public:

	MXS1202();
	void StartSensor();
	void StopSensor();
	int getTempCandHumidity(float *temp,float *humidity);


private:
	static uint16_t    U8FLAG,k;
	static uint8_t    U8count,U8temp;
	static uint8_t    U8T_data_H,U8T_data_L,U8RH_data_H,U8RH_data_L,U8checkdata;

	static uint8_t    U8comdata;
	void COM();
};



#endif
