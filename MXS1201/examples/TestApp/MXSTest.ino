

#include "MXS1101.h"
#include "MXS1201.h"
#include "MXS1301.h"
#include "MXS1401.h"
#include "MXS1501.h"
#include "MXS2101.h"
#include "MXS1202.h"
#include "MXSConfig.h"
#include "MXS4101.h"
#include "MXN820.h"
//The setup function is called once at startup of the sketch
MXS1101 myMXS1101=MXS1101();
MXS1201 myMXS1201=MXS1201();
MXS1301 myMXS1301=MXS1301();
MXS1401 myMXS1401=MXS1401();
MXS1501 myMXS1501=MXS1501();
MXS2101 myMXS2101=MXS2101();
MXS1202 myMXS1202=MXS1202();
MXSConfig myMXSConfig=MXSConfig();
MXS4101 myMXS4101=MXS4101();
MXN820 myMXN820=MXN820();
void MXS1101TEST()
{
	Serial1.write("MXS1101 DS18B20 TEST RESULT:");
	myMXS1101.StartSensor();
	float temp=myMXS1101.getTempC();
	myMXS1101.StopSensor();
	Serial1.print(temp);
}
void MXS1201TEST()
{
	Serial1.write("MXS1201 SHT11 TEST RESULT:");
	myMXS1201.StartSensor();
	float temp=myMXS1201.getTempC();
	float humi=myMXS1201.getHumidity();
	myMXS1201.StopSensor();
	Serial1.write("temp:");
	Serial1.print(temp);
	Serial1.write("      humi:");
	Serial1.print(humi);
	Serial1.write("\n");
}
void MXS1301TEST()
{
	Serial1.write("MXS1301 CO2 TEST RESULT:");
	myMXS1301.StartSensor();
	delay(60000);
	uint16_t temp=myMXS1301.getCO2();
	myMXS1301.StopSensor();
	Serial1.print(temp);
	Serial1.write("\n");
}
void MXS1401TEST()
{
	Serial1.write("MXS1401 SOIL HUMIDTY TEST RESULT:");

	delay(2000);
	int temp=myMXS1401.getSoilHumdity();
	//myMXS1301.StopSensor();
	Serial1.print(temp);
	Serial1.write("\n");
}
void MXS1501TEST()
{
	Serial1.write("MXS1501 Light TEST RESULT:");
	myMXS1501.StartSensor();
	delay(1000);
	double temp=myMXS1501.getLight();
	myMXS1301.StopSensor();
	Serial1.print(temp);
	Serial1.write("\n");
}

void MXS2101TEST()
{
	Serial1.write("MXS2101 CAMERA TEST RESULT:");

	uint32_t size;
	uint16_t count;
	myMXS2101.StartSensor();
	delay(2000);
	myMXS2101.camera_take_picture(CAMERA_SIZE_1, 20, &size, &count);

	uint32_t temp=myMXS2101.camera_get_picture_size();
	myMXS2101.StopSensor();
	Serial1.print(size);
	Serial1.write("\n");
	Serial1.print(count);
	Serial1.write("\n");
}

void MXS1202TEST()
{
	Serial1.write("MXS1202 dht TEST RESULT:");
	float tempc,humidity;
	int result=myMXS1202.getTempCandHumidity(&tempc,&humidity);

	Serial1.print("result:");
	Serial1.print(result);
	Serial1.print("temp:");
	Serial1.print(tempc);
	Serial1.print("Humidity:");
	Serial1.print(humidity);
	Serial1.print("\n");
}
void MXSConfigTest()
{
	byte data[8];
	byte memdata[128];
	Serial1.print("Search Result:");
	Serial1.print(myMXSConfig.SearchAddress(data));
	Serial1.print("\n Search Address:");
	for (int i = 0; i < 8; i++) {
		if (data[i] < 16)
			Serial1.print("0x0");
		else
			Serial1.print("0x");
		Serial1.print(data[i], HEX);
		Serial1.print(" ");
	}
	Serial1.print("\n ");

	Serial1.print(myMXSConfig.ReadAllMem(memdata));
	Serial1.print("\n MEM Value:");
	for (int i = 0; i < 128; i++) {
		if (memdata[i] < 16)
			Serial1.print("0x0");
		else
			Serial1.print("0x");
		Serial1.print(memdata[i], HEX);
		Serial1.print(" ");
	}

	Serial1.write("type");

	Serial1.print(myMXSConfig.GetSensorType());
	Serial1.write("Interval");
	Serial1.print(myMXSConfig.GetSensorinterval());
	Serial1.write("\n");
}
void setup()
{
	// Add your initialization code here
	Serial1.begin(38400);
	Serial1.write("hello world!");
	pinMode(49,OUTPUT);
	digitalWrite(49,LOW);

	myMXN820.BuzzerID();


}

// The loop function is called in an endless loop
void loop()
{
	delay(1000);
	Serial1.print("Sensor Type:");
	Serial1.print(myMXSConfig.GetSensorType());
	Serial1.print("\n");
	Serial1.print(myMXS4101.getValue());
	Serial1.print("\n");
	Serial1.print(myMXS1101.getTempC());



}
