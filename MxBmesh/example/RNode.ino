// Do not remove the include below
#include "RNode.h"
#include "MxRadio.h"
#include "MxBmesh.h"
#include "SerialCrc.h"
#include <FlexiTimer2.h>
#include <util/delay.h>
#define DEBUG 1

#ifndef LOCALADDRESS
#define LOCALADDRESS	1
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
void setup()
{
	MxBmesh.begin(CHANNEL,LOCALADDRESS,15,38400);
	pinMode(30,OUTPUT);
	digitalWrite(29,LOW);
	MxBmesh.setuserintervaluploadfunction(myupload);
	attachInterrupt(0, myhanlder, FALLING);
	attachInterrupt(1, myhanlder, FALLING);
}

void loop()
{
	MxBmesh.poll();
}
