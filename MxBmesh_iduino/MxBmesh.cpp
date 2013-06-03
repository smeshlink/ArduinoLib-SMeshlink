// Do not remove the include below
#include "MxBmesh.h"
#include <util/delay.h>


byte  MXBMESH::retrysend=0;


byte MXBMESH::lastrbseq=0; //last rb seq ,seq start from 255
byte MXBMESH::lasthandledseq[NODECOUNT_MAX]; //last  seq

byte MXBMESH::sendpacketseq=255; //neigbour cast seq
SERIALCRC MXBMESH::serialcrc=SERIALCRC();
QUEUE MXBMESH::rxqueue=QUEUE();
QUEUE MXBMESH::txqueue=QUEUE();
//MXBMESHCONFIG MXBMESH::NODEINFO=MXBMESHCONFIG();
byte MXBMESH::rxserialbuf[SERIALMTU];
byte MXBMESH::rxserialbufindex=0;
uint8_t MXBMESH::hasIntervalElapsedFunction=0;
void (*MXBMESH::IntervalElapsedFunction)();
uint8_t MXBMESH::hasDownloadDataFunction=0;
void (*MXBMESH::DownloadDataFunction)(uint8_t* payload,uint8_t len);
bool MXBMESH::needrecievebroadcastdata=false;
byte MXBMESH::needrecievebroadcastdatadelaycount=0;
bool MXBMESH::needchecksenddone=true;
Timer MXBMESH::broadcast_neigbour_timer(CLOCK_SECOND * NODEINFO.broadcastInterval, true, broadcast_neigbour, NULL);
Timer MXBMESH::data_upload_timer(CLOCK_SECOND * 10, true, data_upload, NULL);

MXBMESH MxBmesh;

MXBMESH::MXBMESH() {
	// TODO Auto-generated constructor stub

}
///+++++++++++++++++++++++++++++++
void MXBMESH::broadcast_neigbour(void *ctx)
{
	for (int nbindex=0;nbindex<MAX_NB;nbindex++)
	{
		if (NODEINFO.neigbour[nbindex].nodeid!=BROADCASTADDR )
		{
			NODEINFO.neigbour[nbindex].intervalcountpast++;
			if (NODEINFO.neigbour[nbindex].intervalcountpast++>BROADCASTINTERVALPAST_MAX)
			{
				NODEINFO.neigbour[nbindex].nodeid=BROADCASTADDR ;
				NODEINFO.neigbour[nbindex].rssi=0 ;
				NODEINFO.neigbour[nbindex].intervalcountpast=0 ;
			}
		}
	}
	if (!needrecievebroadcastdata)
		broadcastdata(MSGTYPE_NB,NULL,0);
}
void MXBMESH::data_upload(void *ctx)
{
	if (hasIntervalElapsedFunction)
		IntervalElapsedFunction();
	else
		uploaddata(MSGTYPE_DU,"bmesh data test!");
}
///+++++++++++++++++++++++++++++++

void MXBMESH::attachDownloadData(void(*funct)(uint8_t* payload,uint8_t len))
{
	DownloadDataFunction=funct;
	hasDownloadDataFunction=(funct == 0) ? 0 : 1;
}
void MXBMESH::attachIntervalElapsed(void(*funct)())
{
	IntervalElapsedFunction=funct;
	hasIntervalElapsedFunction=(funct == 0) ? 0 : 1;
}
void MXBMESH::WriteSerialData(byte *ioData, byte startIndex, int len)
{
	byte crcData[SERIALMTU];
	crcData[0] = (byte)XPACKET_NO_ACK;
	memcpy(crcData+1,ioData+startIndex, len);
	int currentIndex = 0;
	//添加同步字符
	Serial.write((byte)XPACKET_START) ;
	//添加数据包类型
	Serial.write((byte)XPACKET_NO_ACK);
	for (int i = startIndex; i < startIndex + len; i++)
	{
		if (ioData[i] == XPACKET_ESC || ioData[i] == XPACKET_START)
		{
			Serial.write((byte)XPACKET_ESC);
			Serial.write((byte)(ioData[i] ^ 0x20));
		}
		else
		{
			Serial.write((byte) ioData[i]);
		}
	}
	uint16_t crc = serialcrc.crc_packet(crcData,len+1);
	byte crc_lowbyte=crc & 0xff;
	byte crc_highbyte=(crc>>8) & 0xff;

	if (crc_lowbyte == XPACKET_ESC || crc_lowbyte == XPACKET_START)
	{
		Serial.write((byte)XPACKET_ESC);
		Serial.write((byte)(crc_lowbyte ^ 0x20));

	}
	else
	{
		Serial.write((byte)crc_lowbyte );

	}
	if (crc_highbyte == XPACKET_ESC || crc_highbyte == XPACKET_START)
	{
		Serial.write((byte)XPACKET_ESC);
		Serial.write((byte)(crc_highbyte ^ 0x20));
	}
	else
	{
		Serial.write((byte)crc_highbyte );
	}
	Serial.write((byte)XPACKET_START);

}

uint8_t* MXBMESH::recievehandler(uint8_t len, uint8_t* frm, uint8_t lqi, int8_t ed,uint8_t crc_fail)
{
	if (len<INDEX_PATHCOUNT+2 || crc_fail)
		return frm;
	if (frm[INDEX_MSGTYPE]>MSGTYPE_WAKEUP_SHIFT) //get wakeup meesage
		needrecievebroadcastdata=true;
	else
	{
		byte packetindex=rxqueue.inqueue();
		if (packetindex==RFQUENEMAX) //maybe full
			return frm;
		rxqueue.RfData[packetindex].length=len;
		rxqueue.RfData[packetindex].value.rssi=abs(MxRadio.getLastRssi());
		memcpy(rxqueue.RfData[packetindex].rbuf,frm,len);
		needrecievebroadcastdata=false;
	}
	return frm;
}
void MXBMESH::sendbmeshdata(byte nodeid,byte msgtype,const unsigned char *buffer, size_t size,bool withheader)
{
	if (NODEINFO.pathnode[0]==0 && nodeid==SINKADDRESS)
		return; //no path
	byte packdataindex_tx=txqueue.inqueue(); //maybe empty
	byte txbufindex=0;
	if (packdataindex_tx==RFQUENEMAX) return;
	if (nodeid!=BROADCASTADDR)
	{
		if (!withheader)
		{
			txqueue.RfData[packdataindex_tx].value.destaddress=NODEINFO.pathnode[1];

			txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=msgtype;
			txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=NODEINFO.localAddress;
			txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=nodeid;
			txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=sendpacketseq++;
			for (int pathindex=0;pathindex<=NODEINFO.pathnode[0];pathindex++)
			{
				txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=NODEINFO.pathnode[pathindex];
			}
		}
		else
		{
			txqueue.RfData[packdataindex_tx].value.destaddress=nodeid;
		}
		if (size!=0) memcpy(txqueue.RfData[packdataindex_tx].rbuf+txbufindex,buffer,size);
		txqueue.RfData[packdataindex_tx].length=txbufindex+size;
	}
	else
	{
		txqueue.RfData[packdataindex_tx].value.destaddress=0xff;
		if (!withheader)
		{
			txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=msgtype;
			txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=NODEINFO.localAddress;
			txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=BROADCASTADDR;
			txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=sendpacketseq++;
			txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=1;
			txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=BROADCASTADDR;
		}
		if (size!=0)	memcpy(txqueue.RfData[packdataindex_tx].rbuf+txbufindex,buffer,size);
		txqueue.RfData[packdataindex_tx].length=txbufindex+size;
	}
}

void MXBMESH::uploaddata(byte msgtype,const unsigned char *buffer, size_t size)
{
	sendbmeshdata(SINKADDRESS, msgtype,buffer, size);

}
void MXBMESH::uploaddata(byte msgtype,char* str)
{
	sendbmeshdata(SINKADDRESS, msgtype,(const unsigned char *)str, strlen(str));
}
void MXBMESH::broadcastdata(byte msgtype,const uint8_t *buffer, size_t size)
{
	if (msgtype==MSGTYPE_RB)
		sendbmeshdata(BROADCASTADDR,msgtype,buffer,size,true);
	else
		sendbmeshdata(BROADCASTADDR,msgtype,buffer,size);
}
void MXBMESH::broadcastdata(byte msgtype,char* str)
{
	if (msgtype==MSGTYPE_RB)
		sendbmeshdata(BROADCASTADDR, msgtype,(const unsigned char *)str, strlen(str),true);
	else
		sendbmeshdata(BROADCASTADDR, msgtype,(const unsigned char *)str, strlen(str));
}

void MXBMESH::onXmitDone(radio_tx_done_t x)
{

	if (!needchecksenddone) //wakeup not in queue
		return;
	if (x==TX_OK)
	{
		retrysend=0;
		txqueue.dequeue();
	}

	else
	{
		retrysend++;
		if (retrysend>RETRYSEND_MAX )
		{
			txqueue.dequeue();
			retrysend=0;
		}
	}

}
void MXBMESH::begin(channel_t chan,uint16_t _localAddress,char autoretrycount,unsigned long baudrate,bool powermode)
{
	Serial.begin(baudrate);
	NODEINFO.localAddress=_localAddress;
	NODEINFO.localChannel=chan;
	NODEINFO.islowpower=powermode;
	NODEINFO.LoadCONFIG(); //maybe restart

	randomSeed(NODEINFO.localAddress);
	MxRadio.begin(chan,0xffff,NODEINFO.localAddress,true,true,true,autoretrycount);
	MxRadio.setParam(phyTransmitPower,(txpwr_t)-17);

	MxRadio.attachReceiveFrame(recievehandler);
	//MxRadio.attachError(errHandle);
	MxRadio.attachTxDone(onXmitDone);
	for (int nbindex=0;nbindex<MAX_NB;nbindex++)
	{
		NODEINFO.neigbour[nbindex].nodeid=BROADCASTADDR;
	}
	rxqueue.init_queue();
	txqueue.init_queue();


	if (NODEINFO.dataupload_interval==0xff)
		NODEINFO.dataupload_interval=0;
	else
	{
		if (NODEINFO.dataupload_interval!=0)
		{
			data_upload_timer.setTime(CLOCK_SECOND*NODEINFO.dataupload_interval);
			data_upload_timer.start();
		}
	}
	broadcast_neigbour_timer.start();

}
void MXBMESH::serialpackage_send(uint8_t  *buf,byte len)
{
	WriteSerialData(buf,9,len-9-2); //payload start from message type
}

//++++++ only for sinknode
void MXBMESH::serialpackage_recieve(uint8_t  *buf,byte len) //
{
	//need check crc first
	rxserialbufindex=0;
	uint16_t newCrc = serialcrc.crc_packet(buf, 1, len - 4); //crc no 7e
	uint16_t oldCrc =(uint16_t)buf[len-2]*256+buf[len-3];
	//Serial.println(newCrc,DEC);

	//Serial.println(oldCrc,DEC);
	if (len>PACKAGE_MAX-11)
		return;
	byte packetindex_tx;
	switch (buf[2]) //remove 7e42
	{
	case MSGTYPE_NB://no this tyoe data will recieved
	case MSGTYPE_RB:
		broadcastdata(buf[2],buf+2,len-5);
		break;
	case MSGTYPE_CMD_GETPOWER: //sinknode should shift last
		byte sbuf[6];
		sbuf[0]=MSGTYPE_CMD_GETPOWER;
		sbuf[1]=0;
		sbuf[2]=0;
		sbuf[3]=0;
		sbuf[4]=0; //pathcount
		sbuf[5]=NODEINFO.islowpower; //value
		WriteSerialData(sbuf,0, 6);
		break;
	case MSGTYPE_CMD_LOWPOWER:
	case MSGTYPE_CMD_HIGHPOWER:
	case MSGTYPE_CMD_RD:  //download route message
	case MSGTYPE_DD:
	case MSGTYPE_DD_NEEDACK:
	case MSGTYPE_CMD_GETCONFIG:
	case MSGTYPE_DU_ACK:
		if (buf[2]==MSGTYPE_CMD_LOWPOWER)
			NODEINFO.islowpower=true;
		else if (buf[2]==MSGTYPE_CMD_HIGHPOWER)
			NODEINFO.islowpower=false;
		sendbmeshdata(buf[2+INDEX_PATHCOUNT-INDEX_MSGTYPE+1],buf[2],buf+2,len-5,true);
		break;
	}
	//Serial.write(txqueue.RfData[packetindex_tx].value.destaddress);
	//Serial.write(txqueue.RfData[packetindex_tx].rbuf,txqueue.RfData[packetindex_tx].length);
}
void MXBMESH::handlerftx()
{
	////send data
	//_delay_ms(RELAYDELAY);
	byte packdataindex_tx=txqueue.peerqueue(); //maybe empty
	if (packdataindex_tx==RFQUENEMAX) return;
	if (!NODEINFO.islowpower || txqueue.RfData[packdataindex_tx].value.destaddress==SINKADDRESS)
	{
		needchecksenddone=true;
		if (txqueue.RfData[packdataindex_tx].value.destaddress==0xff)
		{
			if (txqueue.RfData[packdataindex_tx].rbuf[0]==MSGTYPE_RB)
				_delay_ms(RELAYDELAY);
			MxRadio.beginTransmission();
		}

		else
		{
			if (txqueue.RfData[packdataindex_tx].rbuf[INDEX_SRCADDR]==NODEINFO.localAddress) //start from me
				_delay_ms(random(RELAYDELAY));
			MxRadio.beginTransmission(txqueue.RfData[packdataindex_tx].value.destaddress);
		}
		MxRadio.write(txqueue.RfData[packdataindex_tx].rbuf,txqueue.RfData[packdataindex_tx].length);
		MxRadio.endTransmission();
	}
	else //send wakeup packet
	{
		if (txqueue.RfData[packdataindex_tx].value.destaddress!=0xff)
		{
			//if (txqueue.RfData[packdataindex_tx].rbuf[INDEX_SRCADDR]==NODEINFO.localAddress &&  NODEINFO.localAddress!=SINKADDRESS)
			//	_delay_ms(random(RELAYDELAY));
			MxRadio.beginTransmission(txqueue.RfData[packdataindex_tx].value.destaddress);
			MxRadio.write(txqueue.RfData[packdataindex_tx].rbuf,txqueue.RfData[packdataindex_tx].length);
			needchecksenddone=true;
			MxRadio.endTransmission();
		}
		else //oxff
		{
			//if (txqueue.RfData[packdataindex_tx].rbuf[0]==MSGTYPE_RB)
			//_delay_ms(WAKEUP_PREAMBLE_MS+random(RELAYDELAY));
			needchecksenddone=false;
			byte msgtype=txqueue.RfData[packdataindex_tx].rbuf[INDEX_MSGTYPE-INDEX_MSGTYPE];
			unsigned long _startMillis = millis();
			do
			{
				MxRadio.beginTransmission();
				txqueue.RfData[packdataindex_tx].rbuf[INDEX_MSGTYPE-INDEX_MSGTYPE]=MSGTYPE_WAKEUP_SHIFT+msgtype;
				MxRadio.write(txqueue.RfData[packdataindex_tx].rbuf,txqueue.RfData[packdataindex_tx].length);
				MxRadio.endTransmission();
			} while(millis() - _startMillis < WAKEUP_PREAMBLE_MS+WAKEUP_PREAMBLE_RELAY_MS);

			MxRadio.beginTransmission();
			txqueue.RfData[packdataindex_tx].rbuf[INDEX_MSGTYPE-INDEX_MSGTYPE]=msgtype;
			MxRadio.write(txqueue.RfData[packdataindex_tx].rbuf,txqueue.RfData[packdataindex_tx].length);
			needchecksenddone=true;
			MxRadio.endTransmission();
		}
	}
	//Serial.write(0);
	//Serial.write(txqueue.RfData[packdataindex_tx].value.destaddress);
	//Serial.write(0);
	//Serial.write(txqueue.RfData[packdataindex_tx].rbuf,txqueue.RfData[packdataindex_tx].length);
}
void MXBMESH::handlerfrx()
{
	byte ackdata[PACKAGE_MAX];
	byte nodeindex=0; //my node index in the path
	byte pathcount=0; //my node to sink path count
	byte packdataindex_rx=0;
	packdataindex_rx=rxqueue.dequeue(); //maybe empty
	if (packdataindex_rx==RFQUENEMAX) return;
	uint16_t destaddress=rxqueue.RfData[packdataindex_rx].rbuf[5]+(uint16_t)rxqueue.RfData[packdataindex_rx].rbuf[6]*256;
	byte srcaddress_last=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_SRCADDR];
	byte destaddress_last=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_DESTADDR];
	uint16_t srcaddress=rxqueue.RfData[packdataindex_rx].rbuf[7]+(uint16_t)rxqueue.RfData[packdataindex_rx].rbuf[8]*256;
	byte messagetype=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_MSGTYPE];
	byte sequenceid=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_SEQUENCE];
	//需要检查一下是否是本协议能处理的数据包，防止第三方数据包对系统造成破坏
	if (destaddress==0xffff)
	{
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_MSGTYPE]!=MSGTYPE_NB && rxqueue.RfData[packdataindex_rx].rbuf[INDEX_MSGTYPE]!=MSGTYPE_RB)
			return;
	}
	else
	{
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_MSGTYPE]<MSGTYPE_RU || rxqueue.RfData[packdataindex_rx].rbuf[INDEX_MSGTYPE]>MSGTYPE_CMD_GETCONFIG)
			return;
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]>MAX_ROUTER || rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]<1) //hop limit
			return;
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_DESTADDR]!=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_DESTADDR+rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]+2] )  //dest address show in two different part
			return;
	}
	//+++++++ check finished
	byte pdataindex=INDEX_DESTADDR+1;
	byte pathindex=0;
	for ( nodeindex =1;nodeindex<=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT];nodeindex++) //nb data is ok ,nodeindex =1
	{
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+nodeindex]==NODEINFO.localAddress || rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+nodeindex]==BROADCASTADDR) //must have this address
			break;
	} //find my address location
	if (nodeindex>MAX_ROUTER) //error package
		return;

	//can't handle same seq message twice
	if (lasthandledseq[srcaddress_last]==sequenceid  )
		return;
	else
		lasthandledseq[srcaddress_last]=sequenceid;

	//relay data
	if (destaddress_last!=NODEINFO.localAddress && destaddress_last!=BROADCASTADDR ) //RB delay is later
	{
		if (rxqueue.RfData[packdataindex_rx].rbuf[nodeindex+1+INDEX_PATHCOUNT]==srcaddress)
			return;//error action
		sendbmeshdata(rxqueue.RfData[packdataindex_rx].rbuf[nodeindex+1+INDEX_PATHCOUNT],messagetype,rxqueue.RfData[packdataindex_rx].rbuf+INDEX_MSGTYPE,rxqueue.RfData[packdataindex_rx].length-INDEX_MSGTYPE-2,true);
		return;
	}
	//relay finished
	//record path
	if (messagetype==MSGTYPE_CMD_RD || messagetype==MSGTYPE_CMD_HIGHPOWER || messagetype==MSGTYPE_CMD_LOWPOWER || messagetype==MSGTYPE_DD || messagetype==MSGTYPE_DD_NEEDACK || messagetype==MSGTYPE_CMD_GETCONFIG)
	{
		pathcount=nodeindex;
		//record path
		for ( nodeindex =1;nodeindex<=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT];nodeindex++)
		{
			NODEINFO.pathnode[pathcount-nodeindex]=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+nodeindex];
			if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+nodeindex]==NODEINFO.localAddress)
				break;
		}
		NODEINFO.pathnode[0]=pathcount;
		NODEINFO.pathnode[pathcount]=SINKADDRESS;
	}
	switch (messagetype)
	{
	case MSGTYPE_RB: //finished
		if (NODEINFO.localAddress!=SINKADDRESS)
		{
			for (int nbindex=0;nbindex<MAX_NB;nbindex++)
			{
				if (NODEINFO.neigbour[nbindex].nodeid==BROADCASTADDR || NODEINFO.neigbour[nbindex].nodeid==srcaddress)
				{
					NODEINFO.neigbour[nbindex].nodeid=srcaddress;
					NODEINFO.neigbour[nbindex].rssi=rxqueue.RfData[packdataindex_rx].value.rssi;
					NODEINFO.neigbour[nbindex].intervalcountpast=0;
					break;
				}
			}

			if ( (uint16_t)((uint16_t)lastrbseq+255)>=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_SEQUENCE] && lastrbseq!=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_SEQUENCE] ) //handle before,every node broadcast one time
			{
				int mynodehop=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT];
				lastrbseq=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_SEQUENCE];
				for ( nodeindex =1;nodeindex<=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT];nodeindex++)
				{
					if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+nodeindex]==BROADCASTADDR) //RB package finish by 255
						break;
					NODEINFO.pathnode[mynodehop-nodeindex]=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+nodeindex];
				}
				NODEINFO.pathnode[0]=nodeindex;
				NODEINFO.pathnode[1]=srcaddress;
				NODEINFO.pathnode[nodeindex]=SINKADDRESS;
				// add my address to broadcast
				rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]]=srcaddress;
				rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]++;
				rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]]=BROADCASTADDR;
				broadcastdata(MSGTYPE_RB,rxqueue.RfData[packdataindex_rx].rbuf+INDEX_MSGTYPE,rxqueue.RfData[packdataindex_rx].length-INDEX_MSGTYPE-2+1);
				//upload neigbourinfo
				//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				//MSGTYPE SRCADDR DESTADDR SEQ PATHCOUNT NODE1 NODE2 NODE3 NEIGBOUR_NODEID RSSI ... CRC
				//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				//+++++++++upload neigbour
				if (NODEINFO.localAddress!=0)
				{
					byte sendbuflen=0;
					for (int neigbourindex=0;neigbourindex<MAX_NB;neigbourindex++)
					{
						if (NODEINFO.neigbour[neigbourindex].nodeid!=BROADCASTADDR)
						{
							ackdata[sendbuflen++]=(byte)NODEINFO.neigbour[neigbourindex].nodeid;
							ackdata[sendbuflen++]=(byte)NODEINFO.neigbour[neigbourindex].rssi;
						}
					}
					uploaddata(MSGTYPE_RU,ackdata,sendbuflen);
				}
				//+++++++++++++++++++

			}
		}
		break;
	case MSGTYPE_CMD_RD: //download route ,every node in the path should handle,last node should ack
		NODEINFO.dataupload_interval=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]+1];
		if (NODEINFO.dataupload_interval<10 && NODEINFO.dataupload_interval>0)
			NODEINFO.dataupload_interval=10;//interval must big than 10s
		data_upload_timer.stop();
		if (NODEINFO.dataupload_interval!=0)
		{
			data_upload_timer.setTime(CLOCK_SECOND*NODEINFO.dataupload_interval);
			data_upload_timer.start();
		}
		NODEINFO.SaveCONFIG();
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]]==NODEINFO.localAddress) //find node should ack
		{
			//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			//MSGTYPE SRCADDR DESTADDR SEQ PATHCOUNT NODE1 NODE2 NODE3...  CRC
			//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			ackdata[0]=messagetype;
			uploaddata(MSGTYPE_CMD_ACK,ackdata,1);
		}
		break;
	case MSGTYPE_CMD_LOWPOWER:
		NODEINFO.islowpower=true;
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]==nodeindex)//本节点是目标节点
		{
			ackdata[0]=messagetype;
			uploaddata(MSGTYPE_CMD_ACK,ackdata,1);
		}
		break;
	case MSGTYPE_CMD_HIGHPOWER:
		NODEINFO.islowpower=false;
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]==nodeindex)//本节点是目标节点
		{
			ackdata[0]=messagetype;
			uploaddata(MSGTYPE_CMD_ACK,ackdata,1);
		}
		break;
	case MSGTYPE_CMD_GETCONFIG:
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]==nodeindex)//本节点是目标节点
		{
			byte sendbuflen=0;
			ackdata[sendbuflen++]=messagetype;
			ackdata[sendbuflen++]=NODEINFO.localAddress;
			ackdata[sendbuflen++]=NODEINFO.localChannel;
			ackdata[sendbuflen++]=NODEINFO.broadcastInterval;
			ackdata[sendbuflen++]=NODEINFO.dataupload_interval;
			ackdata[sendbuflen++]=NODEINFO.islowpower;
			if (NODEINFO.localAddress!=0)
			{
				for (int neigbourindex=0;neigbourindex<MAX_NB;neigbourindex++)
				{
					if (NODEINFO.neigbour[neigbourindex].nodeid!=BROADCASTADDR)
					{
						ackdata[sendbuflen++]=(byte)NODEINFO.neigbour[neigbourindex].nodeid;
						ackdata[sendbuflen++]=(byte)NODEINFO.neigbour[neigbourindex].rssi;
					}
				}
			}
			uploaddata(MSGTYPE_CMD_ACK,ackdata,sendbuflen++);
		}
		break;
	case MSGTYPE_DD:
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]==nodeindex)//本节点是目标节点
		{
			rxqueue.RfData[packdataindex_rx].payloadindex=INDEX_PATHCOUNT+nodeindex+1;
			if (hasDownloadDataFunction)
				DownloadDataFunction(rxqueue.RfData[packdataindex_rx].rbuf+INDEX_PATHCOUNT+rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]+1,rxqueue.RfData[packdataindex_rx].length-(INDEX_PATHCOUNT+rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]+3));
		}
		break;
	case MSGTYPE_DD_NEEDACK:
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]==nodeindex)//本节点是目标节点
		{
			rxqueue.RfData[packdataindex_rx].payloadindex=INDEX_PATHCOUNT+nodeindex+1;
			if (hasDownloadDataFunction)
				DownloadDataFunction(rxqueue.RfData[packdataindex_rx].rbuf+INDEX_PATHCOUNT+rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]+1,rxqueue.RfData[packdataindex_rx].length-(INDEX_PATHCOUNT+rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]+3));
			ackdata[0]=messagetype;
			uploaddata(MSGTYPE_CMD_ACK,ackdata,1);
		}
		break;
	case MSGTYPE_DU://data upload
		if (destaddress_last==SINKADDRESS)//本节点是目标节点
		{
			rxqueue.RfData[packdataindex_rx].payloadindex=INDEX_PATHCOUNT+nodeindex+1;
			serialpackage_send(rxqueue.RfData[packdataindex_rx].rbuf,rxqueue.RfData[packdataindex_rx].length);
		}
		break;
	case MSGTYPE_DU_NEEDACK:
		if (destaddress_last==SINKADDRESS)//本节点是目标节点
		{
			rxqueue.RfData[packdataindex_rx].payloadindex=INDEX_PATHCOUNT+nodeindex+1;
			serialpackage_send(rxqueue.RfData[packdataindex_rx].rbuf,rxqueue.RfData[packdataindex_rx].length);
		}
		break;
	case MSGTYPE_RU: //data upload must forward to base 0

		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]]==SINKADDRESS)//本节点是目标节点
		{
			rxqueue.RfData[packdataindex_rx].payloadindex=INDEX_PATHCOUNT+nodeindex+1;
			//Serial.write(rxqueue.RfData[packdataindex_rx].rbuf,rxqueue.RfData[packdataindex_rx].length);
			serialpackage_send(rxqueue.RfData[packdataindex_rx].rbuf,rxqueue.RfData[packdataindex_rx].length);
		}
		break;

	case MSGTYPE_NB:
		//just for get neigbour info
		byte nbindex;
		for ( nbindex=0;nbindex<MAX_NB;nbindex++)
		{
			if (NODEINFO.neigbour[nbindex].nodeid==BROADCASTADDR || NODEINFO.neigbour[nbindex].nodeid==srcaddress)
			{
				NODEINFO.neigbour[nbindex].nodeid=srcaddress;
				NODEINFO.neigbour[nbindex].rssi=rxqueue.RfData[packdataindex_rx].value.rssi;
				NODEINFO.neigbour[nbindex].intervalcountpast=0;
				break;
			}
		}
		byte tmpbadrssi;
		tmpbadrssi=0;
		byte tmptmpbadnodeidindex;
		if (nbindex==MAX_NB) //remove some worst rssi neigbour
		{
			for ( nbindex=0;nbindex<MAX_NB;nbindex++)
			{
				if (tmpbadrssi<NODEINFO.neigbour[nbindex].rssi)
				{
					tmpbadrssi=NODEINFO.neigbour[nbindex].rssi;
					tmptmpbadnodeidindex=nbindex;
				}
			}
			if (tmpbadrssi>rxqueue.RfData[packdataindex_rx].value.rssi+RSSI_STEP)
			{
				NODEINFO.neigbour[tmptmpbadnodeidindex].nodeid=srcaddress;
				NODEINFO.neigbour[tmptmpbadnodeidindex].rssi=rxqueue.RfData[packdataindex_rx].value.rssi;
				NODEINFO.neigbour[tmptmpbadnodeidindex].intervalcountpast=0;
			}
		}
		break;
	case MSGTYPE_CMD_ACK:
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]==nodeindex)//本节点是目标节点
		{
			rxqueue.RfData[packdataindex_rx].payloadindex=INDEX_PATHCOUNT+nodeindex+1;
			serialpackage_send(rxqueue.RfData[packdataindex_rx].rbuf,rxqueue.RfData[packdataindex_rx].length);
		}
		break;
	case MSGTYPE_DU_ACK:
		break;

	}

}
void MXBMESH::handleserialrx()
{
	while (Serial.available())
	{
		if (rxserialbufindex>SERIALMTU)
			break;
		rxserialbuf[rxserialbufindex]=Serial.read();
		if (rxserialbuf[rxserialbufindex] == 0x7e)
		{
			if (rxserialbuf[0] == 0x7e && rxserialbufindex > 2)//0x7e0x7e,后面一个为开始
			{
				serialpackage_recieve(rxserialbuf, rxserialbufindex + 1);
				rxserialbufindex = 0;
			}
			else if (rxserialbufindex <= 2)
			{
				rxserialbuf[0] = 0x7e;
				rxserialbufindex = 1;
			}
		}
		else
		{
			//XPACKET_ESC为转译符
			if (rxserialbuf[rxserialbufindex] == XPACKET_ESC)
			{
				while (!Serial.available());//wait for next byte get
				rxserialbuf[rxserialbufindex]=Serial.read();
				rxserialbuf[rxserialbufindex] = (byte)(rxserialbuf[rxserialbufindex] ^ 0x20);
			}
			rxserialbufindex++;
		}
	}
}
void MXBMESH::poll()
{

	//处理收到的数据包
	if (NODEINFO.localAddress==SINKADDRESS) //base need handle serial data
	{
		handleserialrx();

	}
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	handlerfrx();
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	handlerftx();
	if (NODEINFO.localAddress==SINKADDRESS)
		return;
	if (txqueue.peerqueue()!=RFQUENEMAX || rxqueue.peerqueue()!=RFQUENEMAX || !NODEINFO.islowpower)
		return;

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	if ((needrecievebroadcastdata && needrecievebroadcastdatadelaycount<=(WAKEUP_PREAMBLE_MS /WAKEUP_STAY_MS+1)))
	{
		needrecievebroadcastdatadelaycount++;

	}
	else
	{
		MxRadio.setState(STATE_SLEEP,1);
		clock_arch_sleepms(WAKEUP_PREAMBLE_MS-WAKEUP_STAY_MS);
		MxRadio.setState(STATE_RXAUTO);
		needrecievebroadcastdata=false;
		needrecievebroadcastdatadelaycount=0;

	}
	_delay_ms(WAKEUP_STAY_MS);

}
