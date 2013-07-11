// Do not remove the include below
#include "RNode.h"
#include "MxBmesh.h"

#ifndef LOCALADDRESS
#define LOCALADDRESS	0

#endif
#define CHANNEL			26

#include <OneWire.h>
#if defined(mx231cc) || defined(mx212cc)
OneWire ds(15);
#define ENCPOWERPIN 8
#define ENCCSPIN 32
#elif defined(iduinorfa1) || defined(iduinorf212) ||  defined(iduinoprorfr2) || defined(iduinoprorf212)
OneWire ds(26);
#define ENCPOWERPIN 8
#define ENCCSPIN 32
#endif

int getmac(byte * mac)
{
	byte i;
	boolean present;
	byte data[8];
	ds.skip();
	present = ds.reset();
	if (present == TRUE){
		ds.write(0x33,0); // Read data command, leave ghost power off
		for ( i = 0; i <8; i++) {
			data[i] = ds.read();
		}
		if (ds.crc8(data,7)==data[7])
		{
			for ( i = 1; i <7; i++)
				mac[i-1]=data[i];
			return 1;

		}
		else
			return 0;
	}
	else
		return 0;

}

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
	Serial.begin(38400);
#ifdef IPSINK
	byte macaddr[6];
	if (!getmac(macaddr))
	{
		delay(250);
		getmac(macaddr);
	}
	pinMode(ENCPOWERPIN,OUTPUT);
	digitalWrite(ENCPOWERPIN,HIGH); //open power
	delay(200);
	digitalWrite(ENCPOWERPIN,LOW); //open power
	uip_ipaddr_t tcpserver;
	uip_ipaddr(tcpserver,192, 168, 1, 100);
	MxBmesh.beginipsink(CHANNEL,LOCALADDRESS,15,macaddr,&tcpserver,80,ENCCSPIN,false);
#endif
	pinMode(30,OUTPUT);
	//MxBmesh.begin(CHANNEL,LOCALADDRESS,15,38400,false);
	MxBmesh.attachIntervalElapsed(myupload);
	MxBmesh.attachDownloadData(mydataget);
	attachInterrupt(0, myhanlder1, FALLING);
	attachInterrupt(1, myhanlder2, FALLING);
	digitalWrite(30,HIGH);
}

void loop()
{
	MxBmesh.poll();

}
