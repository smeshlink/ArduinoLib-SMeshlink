#include "MXS1301.h"
#include "Wire.h"

#define CONTROLCHIPADDRESS  0x27
const uint8_t GET_CO2_CMD[5]={0xFF,0xFE,0x02,0x02,0x03};
char CO2_VALUE[5]={0,0,0,0,0};

MXS1301::MXS1301()
{


}
uint16_t MXS1301::getCO2()
{

	for (int i = 0; i < 5; i++)
		CO2_VALUE[i] = 0;
	Serial.setTimeout(2000);
	Serial.write(GET_CO2_CMD, sizeof(GET_CO2_CMD));
	Serial.readBytes(CO2_VALUE, sizeof(CO2_VALUE));
	//return  (CO2_VALUE[3] << 8) + CO2_VALUE[4];
	if ((byte)CO2_VALUE[0]==0xff && (byte)CO2_VALUE[1]==0xfa  )
		return  (CO2_VALUE[3] << 8) + CO2_VALUE[4];
	else
	{
		Serial.write(GET_CO2_CMD, sizeof(GET_CO2_CMD));
		Serial.readBytes(CO2_VALUE, sizeof(CO2_VALUE));
		if ((byte)CO2_VALUE[0]==0xff && (byte)CO2_VALUE[1]==0xfa  )
			return  (CO2_VALUE[3] << 8) + CO2_VALUE[4];
		else
			return 0;
	}
}

void MXS1301::StartSensor()
{

	Wire.begin(); // set up SEABO I2C support
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x03);
	Wire.write((byte) 0x00);
	Wire.endTransmission(); // leave I2C bus
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x81); //enable 5V and uart0
	Wire.endTransmission(); // leave I2C bus

#if	 defined(isant2400cc) || defined(mx231cc)
	pinMode(28, INPUT); //A4
	pinMode(13, INPUT); //D5
	pinMode(0, INPUT); //B0
#elif	 defined(isant900cb)
	pinMode(31, INPUT); //PB7
	pinMode(35, INPUT); //E3
#endif
	Serial.begin(19200);
}

void MXS1301::StopSensor()
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
