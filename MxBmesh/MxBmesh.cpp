// Do not remove the include below
#include "MxBmesh.h"


#include <FlexiTimer2.h>
#include <util/delay.h>

byte MXBMESH::dataupload_interval=0;
byte MXBMESH::dataupload_interval_count=0;// 0 means no upload data ,others means
byte MXBMESH::broadcast_interval_count=0;
byte  MXBMESH::retrysend=0;


byte MXBMESH::pathnode[MAX_ROUTER];  //start from hop count , next hop, path finish with dest nodeid
byte MXBMESH::lastrbseq=0; //last rb seq ,seq start from 255
byte MXBMESH::lasthandledseq[NODECOUNT_MAX]; //last  seq

byte MXBMESH::sendpacketseq=255; //neigbour cast seq
byte MXBMESH::localaddress=0;
channel_t MXBMESH::localchannel;
SERIALCRC MXBMESH::serialcrc=SERIALCRC();
QUEUE MXBMESH::rxqueue=QUEUE();
QUEUE MXBMESH::txqueue=QUEUE();
neigbourinfo MXBMESH::neigbour[MAX_NB];
byte MXBMESH::rxserialbuf[SERIALMTU];
byte MXBMESH::rxserialbufindex=0;
uint8_t MXBMESH::hasuserintervaluploadfunction=0;
void (*MXBMESH::userintervaluploadfunction)();
MXBMESH MxBmesh;
MXBMESH::MXBMESH() {
	// TODO Auto-generated constructor stub

}
///+++++++++++++++++++++++++++++++
//quene struct
void MXBMESH::setuserintervaluploadfunction(void(*funct)())
{
	userintervaluploadfunction=funct;
	hasuserintervaluploadfunction=(funct == 0) ? 0 : 1;
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
	byte packetindex=rxqueue.inqueue();
	if (packetindex==RFQUENEMAX) //maybe full
		return frm;
	rxqueue.RfData[packetindex].length=len;
	rxqueue.RfData[packetindex].value.rssi=abs(MxRadio.getLastRssi());
	memcpy(rxqueue.RfData[packetindex].rbuf,frm,len);

	return frm;
}
void MXBMESH::uploaddata(byte msgtype,const unsigned char *buffer, size_t size)
{
	if (pathnode[0]==0)
		return; //no path

	byte packdataindex_tx=txqueue.inqueue(); //maybe empty
	byte txbufindex=0;
	if (packdataindex_tx==RFQUENEMAX) return;
	txqueue.RfData[packdataindex_tx].value.destaddress=pathnode[1];
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=msgtype;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=localaddress;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=SINKADDRESS;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=sendpacketseq++;
	for (int pathindex=0;pathindex<=pathnode[0];pathindex++)
	{
		txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=pathnode[pathindex];
		Serial.write(pathnode[pathindex]);
	}
	Serial.write(0xaa);
	Serial.write(buffer,size);
	Serial.write(0xbb);
	Serial.write(size);
	Serial.write(0xcc);
	memcpy(txqueue.RfData[packdataindex_tx].rbuf+txbufindex,buffer,size);
	txqueue.RfData[packdataindex_tx].length=txbufindex+size;

}
void MXBMESH::uploaddata(byte msgtype,char* str)
{
	if (pathnode[0]==0)
		return; //no path

	byte packdataindex_tx=txqueue.inqueue(); //maybe empty
	byte txbufindex=0;
	if (packdataindex_tx==RFQUENEMAX) return;
	txqueue.RfData[packdataindex_tx].value.destaddress=pathnode[1];
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=msgtype;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=localaddress;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=SINKADDRESS;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=sendpacketseq++;
	for (int pathindex=0;pathindex<=pathnode[0];pathindex++)
		txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=pathnode[pathindex];
	memcpy(txqueue.RfData[packdataindex_tx].rbuf+txbufindex,str,strlen(str));
	txqueue.RfData[packdataindex_tx].length=txbufindex+strlen(str);
}
void MXBMESH::broadcastdata(byte msgtype,const uint8_t *buffer, size_t size)
{


	byte packdataindex_tx=txqueue.inqueue(); //maybe empty
	byte txbufindex=0;
	if (packdataindex_tx==RFQUENEMAX) return;
	txqueue.RfData[packdataindex_tx].value.destaddress=0xff;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=msgtype;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=localaddress;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=BROADCASTADDR;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=sendpacketseq++;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=1;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=BROADCASTADDR;
	if (size!=0)	memcpy(txqueue.RfData[packdataindex_tx].rbuf+txbufindex,buffer,size);
	txqueue.RfData[packdataindex_tx].length=txbufindex+size;


}
void MXBMESH::broadcastdata(byte msgtype,char* str)
{
	byte packdataindex_tx=txqueue.inqueue(); //maybe empty
	byte txbufindex=0;
	if (packdataindex_tx==RFQUENEMAX) return;
	txqueue.RfData[packdataindex_tx].value.destaddress=0xff;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=msgtype;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=localaddress;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=BROADCASTADDR;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=sendpacketseq++;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=1;
	txqueue.RfData[packdataindex_tx].rbuf[txbufindex++]=BROADCASTADDR;
	memcpy(txqueue.RfData[packdataindex_tx].rbuf+txbufindex,str,strlen(str));
	txqueue.RfData[packdataindex_tx].length=txbufindex+strlen(str);
}
void MXBMESH::timer2function()
{
	broadcast_interval_count++;
	dataupload_interval_count++;
	if (broadcast_interval_count>BROADCASTINTERVAL / TIMER2TICKS)
	{
		//first check is any negbour removed
		for (int nbindex=0;nbindex<MAX_NB;nbindex++)
		{
			if (neigbour[nbindex].nodeid!=BROADCASTADDR )
			{
				neigbour[nbindex].intervalcountpast++;
				if (neigbour[nbindex].intervalcountpast++>BROADCASTINTERVALPAST_MAX)
				{
					neigbour[nbindex].nodeid=BROADCASTADDR ;
					neigbour[nbindex].rssi=0 ;
					neigbour[nbindex].intervalcountpast=0 ;
				}
			}
		}
		broadcastdata(MSGTYPE_NB,NULL,0);
		broadcast_interval_count=0;
	}
	if (dataupload_interval_count> (dataupload_interval / TIMER2TICKS) && dataupload_interval!=0 ) //upload data
	{
		if (hasuserintervaluploadfunction)
			userintervaluploadfunction();
		else
			uploaddata(MSGTYPE_DU,"helloworld!");

		dataupload_interval_count=0;
	}
}
void MXBMESH::onXmitDone(radio_tx_done_t x)
{

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
void MXBMESH::begin(channel_t chan,uint16_t _localaddress,char autoretrycount,unsigned long baudrate)
{
	//randomSeed(localaddress);
	localaddress=_localaddress;
	localchannel=chan;
	FlexiTimer2::set(TIMER2TICKS*1000, 1.0/1000, timer2function); // call every 30s for broadcast ,resolution must >=1000
	// FlexiTimer2::set(500, flash); // MsTimer2 style is also supported
	FlexiTimer2::start();
	MxRadio.begin(chan,0xffff,localaddress,true,true,true,autoretrycount);
	MxRadio.setParam(phyTransmitPower,(txpwr_t)-17);
	Serial.begin(baudrate);
	MxRadio.attachReceiveFrame(recievehandler);
	//MxRadio.attachError(errHandle);
	MxRadio.attachTxDone(onXmitDone);
	for (int nbindex=0;nbindex<MAX_NB;nbindex++)
	{
		neigbour[nbindex].nodeid=BROADCASTADDR;
	}
	rxqueue.init_queue();
	txqueue.init_queue();
	byte eepromadress=0;
	dataupload_interval=EEPROM.read(eepromadress);
	if (dataupload_interval==0xff)
		dataupload_interval=0;
	pathnode[0]=EEPROM.read(++eepromadress);
	if (pathnode[0]!=0xff)
		for (byte t=1;t<=pathnode[0];t++)
		{
			pathnode[t]=EEPROM.read(++eepromadress );
		}


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
		packetindex_tx=txqueue.inqueue();
		if (packetindex_tx==RFQUENEMAX) //maybe full
			return ;
		txqueue.RfData[packetindex_tx].length=len-5;
		memcpy(txqueue.RfData[packetindex_tx].rbuf,buf+2,len-5);
		txqueue.RfData[packetindex_tx].value.destaddress=0xff;
		break;
	case MSGTYPE_RD:  //download route message
	case MSGTYPE_DD:
		packetindex_tx=txqueue.inqueue();
		if (packetindex_tx==RFQUENEMAX) //maybe full
			return ;
		txqueue.RfData[packetindex_tx].length=len-5;
		memcpy(txqueue.RfData[packetindex_tx].rbuf,buf+2,len-5);
		txqueue.RfData[packetindex_tx].value.destaddress=buf[2+INDEX_PATHCOUNT-INDEX_MSGTYPE+1];

		break;
	}
	//Serial.write(txqueue.RfData[packetindex_tx].value.destaddress);
	//Serial.write(txqueue.RfData[packetindex_tx].rbuf,txqueue.RfData[packetindex_tx].length);
}
void MXBMESH::handlerftx()
{
	////send data
	_delay_ms(RELAYDELAY);
	byte packdataindex_tx=txqueue.peerqueue(); //maybe empty
	if (packdataindex_tx==RFQUENEMAX) return;
	if (txqueue.RfData[packdataindex_tx].value.destaddress==0xff)
		MxRadio.beginTransmission();
	else
		MxRadio.beginTransmission(txqueue.RfData[packdataindex_tx].value.destaddress);
	MxRadio.write(txqueue.RfData[packdataindex_tx].rbuf,txqueue.RfData[packdataindex_tx].length);
	MxRadio.endTransmission();
	//Serial.write(0);
	//Serial.write(txqueue.RfData[packdataindex_tx].value.destaddress);
	//Serial.write(0);
	//Serial.write(txqueue.RfData[packdataindex_tx].rbuf,txqueue.RfData[packdataindex_tx].length);
}
void MXBMESH::handlerfrx()
{
	byte nodeindex=0; //my node index in the path
	byte pathcount=0; //my node to sink path count
	byte packdataindex_rx=0,packdataindex_tx=0;
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
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_MSGTYPE]!=MSGTYPE_RD && rxqueue.RfData[packdataindex_rx].rbuf[INDEX_MSGTYPE]!=MSGTYPE_RU && \
				rxqueue.RfData[packdataindex_rx].rbuf[INDEX_MSGTYPE]!=MSGTYPE_DD && rxqueue.RfData[packdataindex_rx].rbuf[INDEX_MSGTYPE]!=MSGTYPE_DU &&  rxqueue.RfData[packdataindex_rx].rbuf[INDEX_MSGTYPE]!=MSGTYPE_RD_ACK )
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
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+nodeindex]==localaddress || rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+nodeindex]==BROADCASTADDR) //must have this address
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
	if (destaddress_last!=localaddress && destaddress_last!=BROADCASTADDR ) //RB delay is later
	{
		if (rxqueue.RfData[packdataindex_rx].rbuf[nodeindex+1+INDEX_PATHCOUNT]==srcaddress)
			return;//error action
		packdataindex_tx=txqueue.inqueue(); //maybe full
		if (packdataindex_tx==RFQUENEMAX) return;
		txqueue.RfData[packdataindex_tx].length=rxqueue.RfData[packdataindex_rx].length-INDEX_MSGTYPE-2;
		txqueue.RfData[packdataindex_tx].value.destaddress=rxqueue.RfData[packdataindex_rx].rbuf[nodeindex+1+INDEX_PATHCOUNT];
		memcpy(txqueue.RfData[packdataindex_tx].rbuf,rxqueue.RfData[packdataindex_rx].rbuf+INDEX_MSGTYPE,rxqueue.RfData[packdataindex_rx].length-INDEX_MSGTYPE-2);
		return;
	}
	//relay finished
	switch (messagetype)
	{
	case MSGTYPE_RB: //finished
		if (localaddress!=SINKADDRESS)
		{
			for (int nbindex=0;nbindex<MAX_NB;nbindex++)
			{
				if (neigbour[nbindex].nodeid==BROADCASTADDR || neigbour[nbindex].nodeid==srcaddress)
				{
					neigbour[nbindex].nodeid=srcaddress;
					neigbour[nbindex].rssi=rxqueue.RfData[packdataindex_rx].value.rssi;
					neigbour[nbindex].intervalcountpast=0;
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
					pathnode[mynodehop-nodeindex]=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+nodeindex];
				}
				pathnode[0]=mynodehop;
				pathnode[mynodehop]=SINKADDRESS;
				// add my address to broadcast
				rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]]=srcaddress;
				rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]++;
				rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]]=BROADCASTADDR;

				packdataindex_tx=txqueue.inqueue(); //maybe full
				if (packdataindex_tx==RFQUENEMAX) return;
				txqueue.RfData[packdataindex_tx].length=rxqueue.RfData[packdataindex_rx].length-INDEX_MSGTYPE-2+1;
				txqueue.RfData[packdataindex_tx].value.destaddress=0xff;
				memcpy(txqueue.RfData[packdataindex_tx].rbuf,rxqueue.RfData[packdataindex_rx].rbuf+INDEX_MSGTYPE,rxqueue.RfData[packdataindex_rx].length-INDEX_MSGTYPE-2+1);
				//upload neigbourinfo
				//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				//MSGTYPE SRCADDR DESTADDR SEQ PATHCOUNT NODE1 NODE2 NODE3 NEIGBOUR_NODEID RSSI ... CRC
				//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				//+++++++++upload neigbour
				if (localaddress!=0)
				{
					byte sendbuf[MAX_NB*2];
					byte sendbuflen=0;
					for (int neigbourindex=0;neigbourindex<MAX_NB;neigbourindex++)
					{
						if (neigbour[neigbourindex].nodeid!=BROADCASTADDR)
						{
							sendbuf[sendbuflen++]=(byte)neigbour[neigbourindex].nodeid;
							sendbuf[sendbuflen++]=(byte)neigbour[neigbourindex].rssi;
						}
					}
					uploaddata(MSGTYPE_RU,sendbuf,sendbuflen);
				}
				//+++++++++++++++++++

			}
		}
		break;
	case MSGTYPE_RD: //download route ,every node in the path should handle,last node should ack
		pathcount=nodeindex;

		//record path
		for ( nodeindex =1;nodeindex<=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT];nodeindex++)
		{
			pathnode[pathcount-nodeindex]=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+nodeindex];
			if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+nodeindex]==localaddress)
				break;

		}
		pathnode[0]=pathcount;
		pathnode[pathcount]=SINKADDRESS;
		dataupload_interval=rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]+1];
		if (dataupload_interval<10 && dataupload_interval>0)
			dataupload_interval=10;//interval must big than 10s
		byte eepromadress;
		eepromadress=0;
		EEPROM.checkwrite(eepromadress, dataupload_interval);
		for (byte t=0;t<=pathcount;t++)
		{
			EEPROM.checkwrite(++eepromadress, pathnode[t]);
		}
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]]==localaddress) //find node should ack
		{
			//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			//MSGTYPE SRCADDR DESTADDR SEQ PATHCOUNT NODE1 NODE2 NODE3...  CRC
			//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			uploaddata(MSGTYPE_RD_ACK,NULL,0);
		}
		break;
	case MSGTYPE_DD:
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]==nodeindex)//本节点是目标节点
		{
			rxqueue.RfData[packdataindex_rx].payloadindex=INDEX_PATHCOUNT+nodeindex+1;
			serialpackage_send(rxqueue.RfData[packdataindex_rx].rbuf,rxqueue.RfData[packdataindex_rx].length);
		}
		break;
	case MSGTYPE_DU://data upload
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
			if (neigbour[nbindex].nodeid==BROADCASTADDR || neigbour[nbindex].nodeid==srcaddress)
			{
				neigbour[nbindex].nodeid=srcaddress;
				neigbour[nbindex].rssi=rxqueue.RfData[packdataindex_rx].value.rssi;
				neigbour[nbindex].intervalcountpast=0;
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
				if (tmpbadrssi<neigbour[nbindex].rssi)
				{
					tmpbadrssi=neigbour[nbindex].rssi;
					tmptmpbadnodeidindex=nbindex;
				}
			}
			if (tmpbadrssi>rxqueue.RfData[packdataindex_rx].value.rssi+RSSI_STEP)
			{
				neigbour[tmptmpbadnodeidindex].nodeid=srcaddress;
				neigbour[tmptmpbadnodeidindex].rssi=rxqueue.RfData[packdataindex_rx].value.rssi;
				neigbour[tmptmpbadnodeidindex].intervalcountpast=0;
			}
		}
		break;
	case MSGTYPE_RD_ACK:
		if (rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT+rxqueue.RfData[packdataindex_rx].rbuf[INDEX_PATHCOUNT]]==SINKADDRESS)//本节点是目标节点
		{
			rxqueue.RfData[packdataindex_rx].payloadindex=INDEX_PATHCOUNT+nodeindex+1;
			serialpackage_send(rxqueue.RfData[packdataindex_rx].rbuf,rxqueue.RfData[packdataindex_rx].length);
		}
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
	if (localaddress==SINKADDRESS) //base need handle serial data
	{
		handleserialrx();

	}
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	handlerfrx();
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	handlerftx();
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



}
