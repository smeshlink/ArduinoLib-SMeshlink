#ifndef MXN820_h
#define MXN820_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"

class MXN820  {
public:

	MXN820();
	void BuzzerID();
	void BuzzerValue(int value);
	int GetBatteryVoltage();
	int GetChargeVoltage();

private:
	void buzzer_long();
	void buzzer_short();

};



#endif
