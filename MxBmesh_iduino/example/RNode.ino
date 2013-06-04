// Do not remove the include below
#include "RNode.h"
#include "MxBmesh.h"



#ifndef LOCALADDRESS
#define LOCALADDRESS	11
#endif
#define CHANNEL			26
void myupload()
{
	MxBmesh.uploaddata(MSGTYPE_DU,"fuqian");
}
void mydataget(uint8_t *data,uint8_t len)
{
	Serial.write(data,len);

}
void myhanlder1()
{
	digitalWrite(30,!digitalRead(30));

	MxBmesh.uploaddata(MSGTYPE_DU,"BUTTON1 PRESSED");
}
void myhanlder2()
{
	digitalWrite(30,!digitalRead(30));

	MxBmesh.uploaddata(MSGTYPE_DU_NEEDACK,"BUTTON2 PRESSED");
}
void setup()
{
	pinMode(30,OUTPUT);
	MxBmesh.begin(CHANNEL,LOCALADDRESS,15,38400,true);
	MxBmesh.attachIntervalElapsed(myupload);
	MxBmesh.attachDownloadData(mydataget);
	attachInterrupt(0, myhanlder1, FALLING);
	attachInterrupt(1, myhanlder2, FALLING);
	digitalWrite(30,HIGH);
		
}

void loop()
{
	MxBmesh.poll();
	//clock_arch_sleepms(120);
	//		_delay_ms(5);
}
