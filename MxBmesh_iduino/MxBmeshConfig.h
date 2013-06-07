/*
 *  MxBmeshCONFIG.h
 *
 *  Created on: 2013-5-9
 *      Author: fu
 */

#ifndef MXBMESHCONFIG_H_
#define MXBMESHCONFIG_H_
#include <Arduino.h>
#include <EEPROM.h>
#include <MxRadio.h>
#include <psock.h>
#include <NanodeUIP.h>

#define IPSINK			0
#define SERIALSINK		1

#define MAX_NB		25

#define	RETRYSEND_MAX 		10
#define RELAYDELAY		5
#define NODECOUNT_MAX		255
#define IPSERIALMTU	250
#define BROADCASTINTERVALPAST_MAX 4
#define RSSI_STEP 0

#define WAKEUP_PREAMBLE_MS 125
#define WAKEUP_PREAMBLE_RELAY_MS 4
#define WAKEUP_STAY_MS 3

#define SINKADDRESS		0
#define BROADCASTADDR	255

#define MAX_ROUTER		20
#define PACKAGE_MAX		128

#define TXPOWERSHIFT		128

#define	EEPROMSTART 		100
#define	BMESHCONFIGTAG 		0xBB

struct NeigbourInfo
{
	byte nodeid;
	byte rssi;
	byte intervalcountpast;
};
class  MXBMESHCONFIG {
private :

public:
	static byte localAddress;
	static channel_t localChannel;
	static byte broadcastInterval;
	static NeigbourInfo neigbour[MAX_NB];
	static uint16_t dataupload_interval;
	static byte pathnode[MAX_ROUTER];  //start from hop count , next hop, path finish with dest nodeid
	static bool islowpower;
	static byte txPower;

#if IPSINK
	static byte mac[6];
	static uip_ipaddr_t serverip;
	static uint16_t serverport;
#endif
	MXBMESHCONFIG();
	static bool LoadCONFIG();
	static void SaveCONFIG();

};

#endif /* MXBMESHCONFIG_H_ */
