/*
 * MXBMESH.h
 *
 *  Created on: 2013-5-9
 *      Author: fu
 */

#ifndef MXBMESH_H_
#define MXBMESH_H_

#include <MxBmeshConfig.h>
#include <Queue.h>

class MXBMESH {
private:


	static byte retrysend; //retry send count finished
	static byte lastrbseq; //last rb seq ,seq start from 255
	static byte lasthandledseq[NODECOUNT_MAX]; //last rb seq
	static byte sendpacketseq; //send rf package index

	static MXBMESHCONFIG NODEINFO;
	static QUEUE rxqueue;
	static QUEUE txqueue;
	static Timer broadcast_neigbour_timer;
	static Timer data_upload_timer;
	static void broadcast_neigbour(void *ctx);
	static void data_upload(void *ctx);
	static void onXmitDone(radio_tx_done_t x);


#if SINKNODE
	static byte rxserialbuf[SINKMTU];
	static byte rxserialbufindex;
	static void handleserialrx();
	static HardwareSerial *sinkSerial;
	static void WritePacketData(byte *ioData, byte startIndex, int len);
	static void package_recieve(uint8_t  *buf,byte len);
	static void package_send(uint8_t  *buf,byte len);
#endif
	static uint8_t* recievehandler(uint8_t len, uint8_t* frm, uint8_t lqi, int8_t ed,uint8_t crc_fail);
	static uint8_t hasIntervalElapsedFunction;
	static void (*IntervalElapsedFunction)();
	static uint8_t hasDownloadDataFunction;
	static void (*DownloadDataFunction)(uint8_t* payload,uint8_t len);

	static void handlerftx();
	static void handlerfrx();
	static bool needrecievebroadcastdata;
	static byte needrecievebroadcastdatadelaycount;
	/*
	get preamble message ,need recieve real broadcast data ,
	needrecievebroadcastdatadelaycount for time out
	this time it doesn't send broadcast data,p2p data doesn't need it
	*/
	static bool needchecksenddone;



public:
	MXBMESH();
	static void begin(channel_t chan,uint16_t localaddress,HardwareSerial *mySerial,bool powermode=false);
	static void poll();
	static void uploaddata(byte msgtype,const uint8_t *buffer, size_t size);
	static void uploaddata(byte msgtype,char* str);
	static void broadcastdata(byte msgtype,const uint8_t *buffer, size_t size);
	static void broadcastdata(byte msgtype,char* str);
	static void attachIntervalElapsed(void(*)());
	static void attachDownloadData(void(*)(uint8_t* payload,uint8_t len));
	static void sendbmeshdata(byte nodeid,byte msgtype,const unsigned char *buffer, size_t size,bool withheader=false);

};
extern MXBMESH MxBmesh;
#endif /* MXBMESH_H_ */
