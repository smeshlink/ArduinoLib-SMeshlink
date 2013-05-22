#include "MXS4101.h"
#include "Wire.h"

#define CONTROLCHIPADDRESS  0x27
#if	 defined(isant2400cc) || defined(mx231cc)

#define PDATAPIN 13 //PD5
#elif defined(isant900cb)
#define PDATAPIN  31  //Pb7
#elif defined(iduinorfa1)
#define PDATAPIN 24 //PD6
#endif


MXS4101::MXS4101()
{


}

uint8_t MXS4101::getValue() // 1 is not something nearby
{
	uint8_t value=0;
	pinMode(PDATAPIN, INPUT); //D5
	Wire.begin(); // set up SEABO I2C support
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x03);
	Wire.write((byte) 0x00);
	Wire.endTransmission(); // leave I2C bus
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x80); //enable 5V and uart0
	Wire.endTransmission(); // leave I2C bus
	delay(1);
	value=digitalRead(PDATAPIN);
	Wire.begin(); // set up SEABO I2C support
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x03);
	Wire.write((byte) 0x00);
	Wire.endTransmission(); // leave I2C bus
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x00); //enable 5V and uart0
	Wire.endTransmission(); // leave I2C bus
	return value;


}

