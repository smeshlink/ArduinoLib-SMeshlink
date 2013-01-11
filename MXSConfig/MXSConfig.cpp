#include "MXSConfig.h"
#include "Arduino.h"
#include "Wire.h"
#include "OneWire.h"


#if	 defined(isant2400cc) || defined(mx231cc)
#define CONFIGONEWIRE 12 //PD4
#elif defined(isant900cb)
#define CONFIGONEWIRE 5 //PD5
#endif
OneWire ds(CONFIGONEWIRE);
byte MXSConfig::addr[8];
MXSConfig::MXSConfig()
{

}
int MXSConfig::SearchAddress(byte* address) //Search for address and confirm it
{
	if (!ds.search(address)) {
		ds.reset_search();
		delay(250);
		return 0;
	}
	if (OneWire::crc8(address, 7) != address[7]) {
		return 0;
	}

	if (address[0] != 0x2D) {
		return 0;
	}
	for (int i=0;i<8;i++)
		addr[i]=address[i];
	return 1;
}
int MXSConfig::GetSensorInfo(uint8_t sensortype,uint16_t sensorinterval)
{
	if (addr[0] != 0x2D)
	{
		SearchAddress(addr);
		if (addr[0] != 0x2D)
			return 0;
	}
	ds.reset();
	ds.select(addr);
	ds.write(0xF0, 1); // Read Memory
	ds.write(0x00, 1); //Read Offset 0000h
	ds.write(0x00, 1);
	ds.read();
	ds.read();
	sensortype= ds.read();
	sensorinterval=(uint16_t)ds.read()+(uint16_t)ds.read()*256;
	return 1;
}
int MXSConfig::ReadAllMem(byte* memdata,byte size)
{
	int i;
	if (addr[0] != 0x2D)
	{
		SearchAddress(addr);
		if (addr[0] != 0x2D)
			return 0;
	}
	ds.reset();
	ds.select(addr);
	ds.write(0xF0, 1); // Read Memory
	ds.write(0x00, 1); //Read Offset 0000h
	ds.write(0x00, 1);

	for (i = 0; i < size; i++) //whole mem is 144
	{
		memdata[i] = ds.read();

	}
	return 1;

}
uint8_t MXSConfig::GetSensorType()
{
	Serial.write("Value:");
	Serial.write(addr[0]);
	if (addr[0] != 0x2D)
	{
		SearchAddress(addr);

		if (addr[0] != 0x2D)
			return 0;
	}
	ds.reset();
	ds.select(addr);
	ds.write(0xF0, 1); // Read Memory
	ds.write(0x00, 1); //Read Offset 0000h
	ds.write(0x00, 1);
	ds.read();
	ds.read();
	return ds.read();
}
uint16_t MXSConfig::GetSensorinterval()
{
	if (addr[0] != 0x2D)
	{
		SearchAddress(addr);
		if (addr[0] != 0x2D)
			return 0;
	}
	ds.reset();
	ds.select(addr);
	ds.write(0xF0, 1); // Read Memory
	ds.write(0x00, 1); //Read Offset 0000h
	ds.write(0x00, 1);
	ds.read();
	ds.read();
	ds.read();
	return (uint16_t)ds.read()+(uint16_t)ds.read()*256;
}

