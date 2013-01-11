#include "MXS1101.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Wire.h"
#define CONTROLCHIPADDRESS  0x27
#if	 defined(isant2400cc) || defined(mx231cc)
#define ONE_WIRE_BUS 13 //PD5
#elif defined(isant900cb)
#define ONE_WIRE_BUS 31 //PB7
#endif

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature _MXS1101(&oneWire);

MXS1101::MXS1101()
{

}
void MXS1101::StartSensor()
{
	Wire.begin(); // set up SEABO I2C support
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x03);
	Wire.write((byte) 0x00);
	Wire.endTransmission(); // leave I2C bus
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x80); //0x80 enable5V  0x8 disable UART1
	Wire.endTransmission(); // leave I2C bus
#if	 defined(isant2400cc) || defined(mx231cc)
	pinMode(28, OUTPUT); //A4
	digitalWrite(28, HIGH);
#elif defined(isant900cb)
	//pinMode(28, OUTPUT); //A4
	//digitalWrite(28, HIGH);
#endif

}
void MXS1101::StopSensor()
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

}
float MXS1101::getTempC()
{
	oneWire.reset();
	_MXS1101.begin();
	_MXS1101.requestTemperatures();
	return _MXS1101.getTempCByIndex(0);
}
