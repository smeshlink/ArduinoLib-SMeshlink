// Do not remove the include below
#include "MxBMeshConfig.h"
#include "MxRadio.h"
byte MXBMESHCONFIG::localAddress=254;
channel_t MXBMESHCONFIG::localChannel=26;
byte MXBMESHCONFIG::broadcastInterval=90;

uint16_t MXBMESHCONFIG::dataupload_interval=0;
byte MXBMESHCONFIG::pathnode[MAX_ROUTER];  //start from hop count , next hop, path finish with dest nodeid
bool MXBMESHCONFIG::islowpower=false;
#if RADIO_TYPE==RADIO_AT86RF212
byte MXBMESHCONFIG::txPower=8+TXPOWERSHIFT;
#else
byte MXBMESHCONFIG::txPower=3+TXPOWERSHIFT;
#endif
NeigbourInfo MXBMESHCONFIG::neigbour[MAX_NB];

MXBMESHCONFIG::MXBMESHCONFIG() {

}
bool MXBMESHCONFIG::LoadCONFIG()
{
	byte eepromadress,checkvalue;
	eepromadress=EEPROMSTART;
	checkvalue=0;
	if (EEPROM.read(eepromadress++)!=BMESHCONFIGTAG)
	{
		dataupload_interval=0;
		broadcastInterval=90;
		return false;
	}
	checkvalue+=BMESHCONFIGTAG;
	byte _localAddress=EEPROM.read(eepromadress++);
	checkvalue+=_localAddress;
	byte _localChannel=EEPROM.read(eepromadress++);
	checkvalue+=_localChannel;
	byte _broadcastInterval=EEPROM.read(eepromadress++);
	checkvalue+=_broadcastInterval;
	byte _dataupload_interval_low=EEPROM.read(eepromadress++);
	checkvalue+=_dataupload_interval_low;
	byte _dataupload_interval_high=EEPROM.read(eepromadress++);
	checkvalue+=_dataupload_interval_high;
	byte _islowpower=EEPROM.read(eepromadress++);
	checkvalue+=_islowpower;
	byte _txPower=EEPROM.read(eepromadress++);
	checkvalue+=_txPower;

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
		dataupload_interval=_dataupload_interval_low+(uint16_t)_dataupload_interval_high*256;

		islowpower=_islowpower;
		txPower=_txPower;
		for (byte t=0;t<=_pathnode[0];t++)
		{
			pathnode[t]=_pathnode[t];
		}
	}
	else
	{
		dataupload_interval=0;
		broadcastInterval=90;
		return true;
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
	EEPROM.checkwrite(eepromadress++, dataupload_interval& 0xff);
	checkvalue+=dataupload_interval& 0xff;
	EEPROM.checkwrite(eepromadress++, (dataupload_interval >>8)& 0xff); //low byte first
	checkvalue+=(dataupload_interval >>8)& 0xff;
	EEPROM.checkwrite(eepromadress++, islowpower);
	checkvalue+=islowpower;
	EEPROM.checkwrite(eepromadress++, txPower);
	checkvalue+=txPower;

	for (byte t=0;t<=pathnode[0];t++)
	{
		EEPROM.checkwrite(eepromadress++, pathnode[t]);
		checkvalue+=pathnode[t];
	}
	EEPROM.checkwrite(eepromadress++, checkvalue);
}
