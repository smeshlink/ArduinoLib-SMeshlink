// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef SensorConfig_H_
#define SensorConfig_H_
#include "Arduino.h"
//add your includes for the project SensorConfig here
enum{
	Key0=0x30,
	Key1=0x31,
	Key2=0x32,
	Key3=0x33,
	Key4=0x34,
	Key5=0x35,
	Key6=0x36,
	Key7=0x37,
	Key8=0x38,
	KeyY=0x59,
	KeyN=0x4E,
	KeyMax=0x38
};
enum{
	C_MXS1101=1,
	C_MXS1201=2,
	C_MXS1301=3,
	C_MXS1401=4,
	C_MXS1501=5,
	C_MXS2101=6,
	C_MXS2201=7,
	C_MXS4101=8,
	C_MXS1202=9,
};

class MXSConfig  {
public:

	MXSConfig();
	int ReadAllMem(byte* memdata,byte size=128);
	int SearchAddress(byte* address); //must call it first
	uint8_t GetSensorType();
	uint16_t GetSensorinterval();
	int GetSensorInfo(uint8_t sensortype,uint16_t sensorinterval);

private:

	static byte addr[8]; // Contains the eeprom unique ID

};




//add your function definitions for the project SensorConfig here




//Do not add code below this line
#endif /* SensorConfig_H_ */
