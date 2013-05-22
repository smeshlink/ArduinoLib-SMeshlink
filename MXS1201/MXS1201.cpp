#include "MXS1201.h"
#include "Wire.h"
#define CONTROLCHIPADDRESS  0x27
#if	 defined(isant2400cc) || defined(mx231cc)
#define DATAPIN  0  //pb0
#define CLOCKPIN 13 //PD5
#elif defined(isant900cb)
#define DATAPIN  35  //pe3
#define CLOCKPIN 31 //Pb7
#elif defined(iduinorfa1)
#define DATAPIN  23  //pD5
#define CLOCKPIN 25 //PD6
#endif
#include <SHT1x.h>
SHT1x _MXS1201(DATAPIN, CLOCKPIN);

MXS1201::MXS1201()
{

}
void MXS1201::StartSensor()
{


	Wire.begin(); // set up SEABO I2C support
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x03);
	Wire.write((byte) 0x00);
	Wire.endTransmission(); // leave I2C bus
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x00); //disable 5V and uart0
	Wire.endTransmission(); // leave I2C bus
#if	 defined(isant2400cc) || defined(mx231cc)
	pinMode(28, OUTPUT); //PA4
	digitalWrite(28, HIGH);
	pinMode(12, OUTPUT); //PD4 POWER
	digitalWrite(12, HIGH);
	pinMode(2, INPUT);	//PB2
	digitalWrite(2, LOW);
#elif defined(isant900cb)
	pinMode(5, OUTPUT); //PD5 POWER
	digitalWrite(5, HIGH);
#endif

}
void MXS1201::StopSensor()
{
	Wire.begin(); // set up SEABO I2C support
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x03);
	Wire.write((byte) 0x00);
	Wire.endTransmission(); // leave I2C bus
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x00); //disable5V
	Wire.endTransmission(); // leave I2C bus
#if	 defined(isant2400cc) || defined(mx231cc)
	pinMode(12, INPUT);
	pinMode(13, INPUT);
	pinMode(14, INPUT);
	pinMode(15, INPUT);
	pinMode(2, INPUT);
#endif
}
float MXS1201::getTempC()
{
	return _MXS1201.readTemperatureC();
}
float MXS1201::getHumidity()
{
	return _MXS1201.readHumidity();
}

