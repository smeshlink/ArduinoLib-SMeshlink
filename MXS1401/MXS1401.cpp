#include "MXS1401.h"
#include "Wire.h"
#define CONTROLCHIPADDRESS  0x27


MXS1401::MXS1401()
{
}
int MXS1401::getSoilHumdity()
{
	return min((analogRead(A2) / 682.00 * 100),100);
}

void MXS1401::StartSensor()
{
	Wire.begin(); // set up SEABO I2C support
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x03);
	Wire.write((byte) 0x00);
	Wire.endTransmission(); // leave I2C bus
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x80); //enable 5V  disable uart1
	Wire.endTransmission(); // leave I2C bus
#if	 defined(isant2400cc) || defined(mx231cc)
	pinMode(28, OUTPUT); //A4
	pinMode(13, OUTPUT); //D5
	//pinMode(0, OUTPUT); //B0
	digitalWrite(28,LOW);
	digitalWrite(13,LOW);
	delay(100);
	pinMode(28, INPUT); //A4
	pinMode(13, INPUT); //D5
	pinMode(A2, INPUT);
	analogReference(1); //ref vcc is 3v
#elif defined(isant900cb)
	pinMode(31, INPUT); //PB7

	analogReference(1);
#endif

}

void MXS1401::StopSensor()
{
	Wire.begin(); // set up SEABO I2C support
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x03);
	Wire.write((byte) 0x00);
	Wire.endTransmission(); // leave I2C bus
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x00); //enable 5V uart1
	Wire.endTransmission(); // leave I2C bus

}
