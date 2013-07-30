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
#include <MxTimer2.h>
#include "stdint.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//MSGTYPE SRCADDR DESTADDR SEQ PATHCOUNT NODE1 NODE2 NODE3... PAYLOAD CRC
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define WITHSINKSOFT		1
#if WITHSINKSOFT
#include <SerialCrc.h>
#define SINKMTU			256 	//250 serial max size
enum HDLCPacketType
{
	XPACKET_ACK = 0x40,
	XPACKET_W_ACK = 0x41,
	XPACKET_NO_ACK = 0x42,

	XPACKET_ESC = 0x7D,    //!< Reserved for serial packetizer escape code.
	XPACKET_START = 0x7E,    //!< Reserved for serial packetizer start code.
	XPACKET_TEXT_MSG = 0xF8,    //!< Special id for sending text error messages.
};
#endif

#define PACKAGE_MAX		128  //rf package max size

#if (RAMEND < 8000) 		//rf quene size rxquene=txquene
#define	RFQUENEMAX 		10
#elif (RAMEND < 16000)
#define	RFQUENEMAX 		20
#else
#define	RFQUENEMAX 		35
#endif

#define MAX_NB		20	//neigbour size

#define	RETRYSEND_MAX 		10  //auto resend max count
#define RELAYDELAY		5		//relay rf delay
#define NODECOUNT_MAX		254	//255 //max node size
#define BROADCASTINTERVALPAST_MAX 4 //neigbour lost count
#define RSSI_STEP 0				//replace neigbour node rssi good

#define WAKEUP_PREAMBLE_MS 125	//wakeup preamble send time 1
#define WAKEUP_PREAMBLE_RELAY_MS 4 //wakeup preamble send time 2 1+2 is realy send time
#define WAKEUP_STAY_MS 3	//node wakeup time

#define SINKADDRESS		0
#define BROADCASTADDR	255

#define MAX_ROUTER		15 //hop limit


#define TXPOWERSHIFT		128 //eeprom value is real txpower+TXPOWERSHIFT

#define	EEPROMSTART 		0
#define	BMESHCONFIGTAG 		0xBB //bmesh config start tag


#define MSGTYPE_NB		1 //neighbour broadcast
#define MSGTYPE_RB		2  //start route ,start from base broadcast
#define MSGTYPE_RU		3   //neigbour info up
#define MSGTYPE_RU_ACK	4
#define MSGTYPE_DU		5
#define MSGTYPE_DU_NEEDACK		6
#define MSGTYPE_DU_ACK	7
#define MSGTYPE_DD		8   //payload format start from data type 0 means collect 1-255 means auto upload interval
#define MSGTYPE_DD_NEEDACK		9   //payload format start from data type 0 means collect 1-255 means auto upload interval

#define MSGTYPE_CMD_ACK 10   //commd down ack
#define MSGTYPE_CMD_RD	11   //route down select longest path,enable all the node in the path can recieve
#define MSGTYPE_CMD_LOWPOWER     12   //commd down
#define MSGTYPE_CMD_HIGHPOWER    13   //commd down
#define MSGTYPE_CMD_GETBASEINFO     14   //commd down
#define MSGTYPE_CMD_GETCONFIG    15   //commd down
#define MSGTYPE_CMD_SETCONFIG    16   //commd down

#define MSGTYPE_CMD_CLRNEIGBOUR    17   //commd down
#define MSGTYPE_CMD_REBOOT    18   //commd down

#define MSGTYPE_DU_NEEDAPPACK		19 //data upload

#define MSGTYPE_CMD_SETBASEROUTE		20 //data get
#define MSGTYPE_CMD_GETDATA		21 //data get

#define MSGTYPE_WAKEUP_SHIFT		0xF0 //wakeup preamble message type

#define INDEX_MSGTYPE		9
#define INDEX_SRCADDR		10
#define INDEX_DESTADDR		11
#define INDEX_SEQUENCE		12
#define INDEX_PATHCOUNT		13

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


	MXBMESHCONFIG();
	static bool LoadCONFIG();
	static void SaveCONFIG();

};

#endif /* MXBMESHCONFIG_H_ */
