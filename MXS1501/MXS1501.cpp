#include "MXS1501.h"
#include "Wire.h"
#include "softI2C.h"
#define CONTROLCHIPADDRESS  0x27
#if	 defined(isant2400cc) || defined(mx231cc)
#define DATAPIN  0  //pb0
#define CLOCKPIN 13 //PD5
#elif defined(isant900cb)
#define DATAPIN  35  //pe3
#define CLOCKPIN 31 //Pb7
#endif
#define MXS1501ADDRESS  0x4A
softI2C _MXS1501(DATAPIN, CLOCKPIN, MXS1501ADDRESS);


MXS1501::MXS1501()
{
}
float MXS1501::getLight()
{

	int x = _MXS1501.readReg(0x03, 0);
	int y = _MXS1501.readReg(0x04, 0);
	byte xevalue = (x >> 4) & 0x0F;
	byte xtvalue = (x) & 0x0F;
	byte ytvalue = y & 0x0F;
	float xvalue = (pow(2, xevalue)) * xtvalue * 0.72;
	float yvalue = (pow(2, xevalue)) * ytvalue * 0.045;
	return (xvalue + yvalue);
}

void MXS1501::StartSensor()
{
	Wire.begin(); // set up SEABO I2C support
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x03);
	Wire.write((byte) 0x00);
	Wire.endTransmission(); // leave I2C bus
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x00); //disable 5V uart1
	Wire.endTransmission(); // leave I2C bus
#if	 defined(isant2400cc) || defined(mx231cc)
	pinMode(28, OUTPUT); //PA4
	digitalWrite(28, HIGH);
	pinMode(12, OUTPUT); //PD4
	digitalWrite(12, HIGH);
	pinMode(2, INPUT);	//PB2
	digitalWrite(2, LOW);
#elif defined(isant900cb)
	pinMode(5, OUTPUT); //PD5
	digitalWrite(5, HIGH);

#endif
	_MXS1501.begin();
	_MXS1501.writeReg(0x02, 0x83, 0);
}

void MXS1501::StopSensor()
{
	Wire.begin(); // set up SEABO I2C support
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x03);
	Wire.write((byte) 0x00);
	Wire.endTransmission(); // leave I2C bus
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x00); //disable 5V uart1
	Wire.endTransmission(); // leave I2C bus

}
