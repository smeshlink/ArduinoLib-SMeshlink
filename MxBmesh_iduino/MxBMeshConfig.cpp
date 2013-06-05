// Do not remove the include below
#include "MxBMeshConfig.h"

byte MXBMESHCONFIG::localAddress=254;
channel_t MXBMESHCONFIG::localChannel=26;
byte MXBMESHCONFIG::broadcastInterval=90;
byte MXBMESHCONFIG::dataupload_interval=0;
byte MXBMESHCONFIG::pathnode[MAX_ROUTER];  //start from hop count , next hop, path finish with dest nodeid
bool MXBMESHCONFIG::islowpower=false;
bool MXBMESHCONFIG::isIpSink=false;
NeigbourInfo MXBMESHCONFIG::neigbour[MAX_NB];
#if IPSINK
byte MXBMESHCONFIG::mac[6];
uip_ipaddr_t MXBMESHCONFIG::serverip;
uint16_t MXBMESHCONFIG::serverport=80;
#endif
MXBMESHCONFIG::MXBMESHCONFIG() {

}
void MXBMESHCONFIG::LoadCONFIG()
{
	byte eepromadress,checkvalue;
	eepromadress=EEPROMSTART;
	checkvalue=0;
	if (EEPROM.read(eepromadress++)!=BMESHCONFIGTAG)
	{
		dataupload_interval=0;
		broadcastInterval=90;
			return;
	}
	checkvalue+=BMESHCONFIGTAG;
	byte _localAddress=EEPROM.read(eepromadress++);
	checkvalue+=_localAddress;
	byte _localChannel=EEPROM.read(eepromadress++);
	checkvalue+=_localChannel;
	byte _broadcastInterval=EEPROM.read(eepromadress++);
	checkvalue+=_broadcastInterval;
	byte _dataupload_interval=EEPROM.read(eepromadress++);
	checkvalue+=_dataupload_interval;
	byte _islowpower=EEPROM.read(eepromadress++);
	checkvalue+=_islowpower;
	byte _pathnode[MAX_ROUTER];
	_pathnode[0]=EEPROM.read(eepromadress++);
	checkvalue+=_pathnode[0];
	for (byte t=1;t<=_pathnode[0];t++)
	{
		_pathnode[t]=EEPROM.read(eepromadress++);
		checkvalue+=_pathnode[t];
	}
	if (checkvalue==EEPROM.read(eepromadress++))
	{
		localAddress=_localAddress;
		localChannel=_localChannel;
		broadcastInterval=_broadcastInterval;
		dataupload_interval=_dataupload_interval;
		islowpower=_islowpower;
		for (byte t=0;t<=_pathnode[0];t++)
		{
			pathnode[t]=_pathnode[t];
		}
	}
	else
	{
		dataupload_interval=0;
		broadcastInterval=90;
		return;
	}
}
void MXBMESHCONFIG::SaveCONFIG()
{
	byte eepromadress,checkvalue;
	eepromadress=EEPROMSTART;
	checkvalue=0;
	EEPROM.checkwrite(eepromadress++, BMESHCONFIGTAG);
	checkvalue+=BMESHCONFIGTAG;
	EEPROM.checkwrite(eepromadress++, localAddress);
	checkvalue+=localAddress;
	EEPROM.checkwrite(eepromadress++, localChannel);
	checkvalue+=localChannel;
	EEPROM.checkwrite(eepromadress++, broadcastInterval);
	checkvalue+=broadcastInterval;
	EEPROM.checkwrite(eepromadress++, dataupload_interval);
	checkvalue+=dataupload_interval;
	EEPROM.checkwrite(eepromadress++, islowpower);
	checkvalue+=islowpower;
	for (byte t=0;t<=pathnode[0];t++)
	{
		EEPROM.checkwrite(eepromadress++, pathnode[t]);
		checkvalue+=pathnode[t];
	}
	EEPROM.checkwrite(eepromadress++, checkvalue);
}
