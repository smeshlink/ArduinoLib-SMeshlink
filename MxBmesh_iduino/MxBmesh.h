/*
 * MXBMESH.h
 *
 *  Created on: 2013-5-9
 *      Author: fu
 */

#ifndef MXBMESH_H_
#define MXBMESH_H_
#include "contiki.h"
#include <Arduino.h>
#include <MxRadio.h>
#include <Timer.h>
#include <Queue.h>
#include <EEPROM.h>
#include <Process.h>
#include <SerialCrc.h>

#define DEBUG 1
#define	RETRYSEND_MAX 		10
#define	BROADCAST_COUNT 	60

#define RELAYDELAY		5
#define NODECOUNT_MAX		255
#define SERIALMTU	256
#define BROADCASTINTERVAL 90
#define TIMER2TICKS 10 //this is timer period
#define BROADCASTINTERVALPAST_MAX 4
#define RSSI_STEP 0

#define WAKEUP_PREAMBLE_MS 125
#define WAKEUP_PREAMBLE_RELAY_MS 4
#define WAKEUP_STAY_MS 3

#define SINKADDRESS		0

#define BROADCASTADDR	255
#define MSGTYPE_NB		1 //neighbour broadcast
#define MSGTYPE_RB		2  //start route ,start from base broadcast
#define MSGTYPE_RU		3   //neigbour info up
#define MSGTYPE_RU_ACK	4
#define MSGTYPE_DU		5
#define MSGTYPE_DU_ACK	6
#define MSGTYPE_DD		7   //payload format start from data type 0 means collect 1-255 means auto upload interval
#define MSGTYPE_DD_NEEDACK		8   //payload format start from data type 0 means collect 1-255 means auto upload interval

#define MSGTYPE_CMD_ACK 10   //commd down ack
#define MSGTYPE_CMD_RD	11   //route down select longest path,enable all the node in the path can recieve
#define MSGTYPE_CMD_LOWPOWER     12   //commd down
#define MSGTYPE_CMD_HIGHPOWER    13   //commd down
#define MSGTYPE_CMD_GETPOWER     14   //commd down

#define MSGTYPE_WAKEUP_SHIFT		0xF0

#define INDEX_MSGTYPE		9
#define INDEX_SRCADDR		10
#define INDEX_DESTADDR		11
#define INDEX_SEQUENCE		12
#define INDEX_PATHCOUNT		13
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//MSGTYPE SRCADDR DESTADDR SEQ PATHCOUNT NODE1 NODE2 NODE3... PAYLOAD CRC
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define MAX_NB		25
#define MAX_ROUTER		20
#define PACKAGE_MAX		128
#define	RFQUENEMAX 		50


struct neigbourinfo
{
	byte nodeid;
	byte rssi;
	byte intervalcountpast;
};
enum HDLCPacketType
{
	XPACKET_ACK = 0x40,
	XPACKET_W_ACK = 0x41,
	XPACKET_NO_ACK = 0x42,

	XPACKET_ESC = 0x7D,    //!< Reserved for serial packetizer escape code.
	XPACKET_START = 0x7E,    //!< Reserved for serial packetizer start code.
	XPACKET_TEXT_MSG = 0xF8,    //!< Special id for sending text error messages.
};

class MXBMESH {
private:

	static byte dataupload_interval;
	static byte dataupload_interval_count;// 0 means no upload data ,others means
	static byte broadcast_interval_count;
	static byte  retrysend;


	static byte pathnode[MAX_ROUTER];  //start from hop count , next hop, path finish with dest nodeid
	static byte lastrbseq; //last rb seq ,seq start from 255
	static byte lasthandledseq[NODECOUNT_MAX]; //last rb seq

	static byte sendpacketseq; //neigbour cast seq
	static byte localaddress;
	static channel_t localchannel;
	static SERIALCRC serialcrc;
	static QUEUE rxqueue;
	static QUEUE txqueue;
	static Timer broadcast_neigbour_timer;
	static Timer data_upload_timer;
	static void broadcast_neigbour(void *ctx);
	static void data_upload(void *ctx);
	static void timer2function();
	static void WriteSerialData(byte *ioData, byte startIndex, int len);
	static void onXmitDone(radio_tx_done_t x);
	static void serialpackage_send(uint8_t  *buf,byte len);
	static void serialpackage_recieve(uint8_t  *buf,byte len);

		///+++++++++++++++++++++++++++++++
	static neigbourinfo neigbour[MAX_NB];

	static byte rxserialbuf[SERIALMTU];
	static byte rxserialbufindex;

	static uint8_t* recievehandler(uint8_t len, uint8_t* frm, uint8_t lqi, int8_t ed,uint8_t crc_fail);
	static uint8_t hasIntervalElapsedFunction;
	static void (*IntervalElapsedFunction)();
	static uint8_t hasDownloadDataFunction;
	static void (*DownloadDataFunction)(uint8_t* payload,uint8_t len);

	static void handlerftx();
	static void handlerfrx();
	static void handleserialrx();
	static bool islowpower;
	static bool needrecievebroadcastdata;
	static bool needchecksenddone;
	static byte needrecievebroadcastdatadelaycount;
public:
	MXBMESH();
	static void begin(channel_t chan,uint16_t localaddress,char autoretrycount,unsigned long baudrate,bool powermode=false);
	static void poll();
	static void uploaddata(byte msgtype,const uint8_t *buffer, size_t size);
	static void uploaddata(byte msgtype,char* str);
	static void broadcastdata(byte msgtype,const uint8_t *buffer, size_t size);
	static void broadcastdata(byte msgtype,char* str);
	static void attachIntervalElapsed(void(*)());
	static void attachDownloadData(void(*)(uint8_t* payload,uint8_t len));
};
extern MXBMESH MxBmesh;
#endif /* MXBMESH_H_ */
