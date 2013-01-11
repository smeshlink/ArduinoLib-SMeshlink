#include "MXS2201.h"
#include "Wire.h"
#define CONTROLCHIPADDRESS  0x27
#include "TinyGPS.h"
TinyGPS gps;
HardwareSerial MXS2201::_mySerial=Serial;
MXS2201::MXS2201(HardwareSerial mySerial)
{
	_mySerial=mySerial;
	_mySerial.begin(9600);
}

MXS2201::MXS2201()
{
	_mySerial.begin(9600);
}
void MXS2201::get_info(float *latitude, float *longitude, float *altitude,float *speed_mps,int *year, byte *month, byte *day,
		byte *hour, byte *minute, byte *second,byte *hundredths,unsigned long *fix_age)
{
	bool newData = false;


	// For one second we parse GPS data and report some key values
	for (unsigned long start = millis(); millis() - start < 1500;)
	{
		while (_mySerial.available())
		{
			char c = _mySerial.read();
			if (gps.encode(c)) // Did a new valid sentence come in?
				newData = true;
		}
	}

	if (newData)
	{
		gps.f_get_position(latitude,longitude,fix_age);
		gps.crack_datetime(year, month, day,
				hour, minute, second,hundredths,fix_age );
		*altitude=gps.f_altitude();
		*speed_mps=gps.f_speed_mps();
		return;
	}
}


void MXS2201::StartSensor()
{
	Wire.begin(); // set up SEABO I2C support
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x03);
	Wire.write((byte) 0x00);
	Wire.endTransmission(); // leave I2C bus
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x80); //enable 5V uart1
	Wire.endTransmission(); // leave I2C bus
#if	 defined(isant2400cc) || defined(mx231cc)
	pinMode(12, INPUT); //D4 as interface
	pinMode(13, INPUT); //D4 d5 d6 d7 input
	pinMode(14, INPUT); //D4 d5 d6 d7 input
	pinMode(15, INPUT); //D4 d5 d6 d7 input
#endif

}

void MXS2201::StopSensor()
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
