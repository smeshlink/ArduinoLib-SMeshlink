// Do not remove the include below
#include "RNode.h"
#include "MxBmesh.h"

#define DEBUG 1

#ifndef LOCALADDRESS
#define LOCALADDRESS	6
#endif
#define CHANNEL			26

void myupload()
{
	MxBmesh.uploaddata(MSGTYPE_DU,"fuqian");
}
void myhanlder()
{
	digitalWrite(30,!digitalRead(30));
	MxBmesh.uploaddata(MSGTYPE_DU,"ERROR");
}
void mydataget(uint8_t *data,uint8_t len)
{
	Serial.write(data,len);
}
void setup()
{
	Serial.begin(38400);


	MxBmesh.begin(CHANNEL,LOCALADDRESS,&Serial,true);
	//pinMode(30,OUTPUT);
	//digitalWrite(29,LOW);
	MxBmesh.attachIntervalElapsed(myupload);
	MxBmesh.attachDownloadData(mydataget);
	attachInterrupt(0, myhanlder, FALLING);
	attachInterrupt(1, myhanlder, FALLING);
}

void loop()
{
	MxBmesh.poll();
}
