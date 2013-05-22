// Do not remove the include below
#include "MxBmesh.h"


#include <FlexiTimer2.h>
#include <util/delay.h>

byte MXBMESH::dataupload_interval=0;
byte MXBMESH::dataupload_interval_count=0;// 0 means no upload data ,others means
byte MXBMESH::broadcast_interval_count=0;
byte  MXBMESH::packdataindex=RXQUENEMAX;
byte  MXBMESH::retrysend=0;


byte MXBMESH::pathnode[MAX_ROUTER];  //start from hop count , next hop, path finish with dest nodeid
byte MXBMESH::lastrbseq=0; //last rb seq ,seq start from 255
byte MXBMESH::lasthandledseq[NODECOUNT_MAX]; //last rb seq

byte MXBMESH::sendpacketseq=255; //neigbour cast seq
byte MXBMESH::localaddress;
channel_t MXBMESH::localchannel;
SERIALCRC MXBMESH::serialcrc=SERIALCRC();
Queue MXBMESH::rxqueue;

neigbourinfo MXBMESH::neigbour[MAX_NB];

byte MXBMESH::rxserialbuf[SERIALMTU];
byte MXBMESH::rxserialbufindex;
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
void MXBMESH::init_queue()
{
	rxqueue.front=rxqueue.rear=0;
}
byte MXBMESH::inqueue()
{
	if((rxqueue.rear+1)%RXQUENEMAX==rxqueue.front)//队列满
	{
		return RXQUENEMAX ;
	}
	else//非满
	{
		byte t=rxqueue.rear;
		rxqueue.rear=(rxqueue.rear+1)%RXQUENEMAX;
		return t;
	}
}
byte MXBMESH::dequeue()
{
	RxData temp;
	if(rxqueue.front==rxqueue.rear)//队列是空
	{
		return RXQUENEMAX;
	}
	byte t=rxqueue.front;
	rxqueue.front=(rxqueue.front+1) % RXQUENEMAX;
	return t;
}
void MXBMESH::undodequeue() //used for no ack get
{
	rxqueue.front=(rxqueue.front+RXQUENEMAX-1) % RXQUENEMAX;
	return ;
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
	byte packetindex=inqueue();
	if (packetindex==RXQUENEMAX) //maybe full
		return frm;
	rxqueue.rxData[packetindex].length=len;
	rxqueue.rxData[packetindex].rssi=ed;
	memcpy(rxqueue.rxData[packetindex].rbuf,frm,len);

	return frm;
}
void MXBMESH::uploaddata(byte msgtype,const unsigned char *buffer, size_t size)
{
	if (pathnode[0]==0)
		return; //no path
	MxRadio.beginTransmission(pathnode[1]);
	MxRadio.write((byte)msgtype);
	MxRadio.write((byte)localaddress);
	MxRadio.write((byte)SINKADDRESS);
	MxRadio.write(sendpacketseq++);
	for (int pathindex=0;pathindex<=pathnode[0];pathindex++)
		MxRadio.write((byte)pathnode[pathindex]);
	MxRadio.write((unsigned char *)buffer,size);
	MxRadio.endTransmission();

}
void MXBMESH::uploaddata(byte msgtype,char* str)
{
	if (pathnode[0]==0)
		return; //no path
	MxRadio.beginTransmission(pathnode[1]);
	MxRadio.write((byte)msgtype);
	MxRadio.write((byte)localaddress);
	MxRadio.write((byte)SINKADDRESS);
	MxRadio.write(sendpacketseq++);
	for (int pathindex=0;pathindex<=pathnode[0];pathindex++)
		MxRadio.write((byte)pathnode[pathindex]);
	MxRadio.write(str);
	MxRadio.endTransmission();

}
void MXBMESH::broadcastdata(byte msgtype,const uint8_t *buffer, size_t size)
{
	//now broadcast message
	MxRadio.beginTransmission();
	MxRadio.write((byte)MSGTYPE_NB);
	MxRadio.write((byte)localaddress);
	MxRadio.write((byte)BROADCASTADDR);
	MxRadio.write(sendpacketseq++);
	MxRadio.write(1); //pathcount
	MxRadio.write((byte)BROADCASTADDR);//just for same with other data format
	if (size!=0)
		MxRadio.write((unsigned char *)buffer,size);
	MxRadio.endTransmission();

}
void MXBMESH::broadcastdata(byte msgtype,char* str)
{
	//now broadcast message
	MxRadio.beginTransmission();
	MxRadio.write((byte)MSGTYPE_NB);
	MxRadio.write((byte)localaddress);
	MxRadio.write((byte)BROADCASTADDR);
	MxRadio.write(sendpacketseq++);
	MxRadio.write(1); //pathcount
	MxRadio.write((byte)BROADCASTADDR);//just for same with other data format
	MxRadio.write(str);
	MxRadio.endTransmission();

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
	if (x==TX_NO_ACK)
	{
		retrysend++;
		if (retrysend<RETRYSEND_MAX )
			undodequeue();
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
	Serial.begin(baudrate);
	MxRadio.attachReceiveFrame(recievehandler);
	//MxRadio.attachError(errHandle);
	MxRadio.attachTxDone(onXmitDone);
	for (int nbindex=0;nbindex<MAX_NB;nbindex++)
	{
		neigbour[nbindex].nodeid=BROADCASTADDR;
	}
	init_queue();

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
	switch (buf[2]) //remove 7e42
	{
	case MSGTYPE_NB://no this tyoe data will recieved
	case MSGTYPE_RB:
		MxRadio.beginTransmission();
		MxRadio.write(buf+2,len-5);
		MxRadio.endTransmission();
		//lastrbseq=buf[2+INDEX_SEQUENCE-INDEX_MSGTYPE];  //forbid base rebroadcast again
		break;
	case MSGTYPE_RD:  //download route message
	case MSGTYPE_DD:
		MxRadio.beginTransmission(buf[2+INDEX_PATHCOUNT-INDEX_MSGTYPE+1]);
		MxRadio.write(buf+2,len-5);
		MxRadio.endTransmission();
		//lastrbseq=buf[2+INDEX_SEQUENCE-INDEX_MSGTYPE];  //forbid base rebroadcast again
		break;
	}
}

void MXBMESH::poll()
{
	//处理收到的数据包
	if (localaddress==SINKADDRESS) //base need handle serial data
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

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	int	nodeindex; //my node index in the path
	int pathcount; //my node to sink path count
	packdataindex=dequeue(); //maybe empty
	if (packdataindex==RXQUENEMAX) return;

	uint16_t destaddress=rxqueue.rxData[packdataindex].rbuf[5]+(uint16_t)rxqueue.rxData[packdataindex].rbuf[6]*256;
	byte srcaddress_last=rxqueue.rxData[packdataindex].rbuf[INDEX_SRCADDR];
	byte destaddress_last=rxqueue.rxData[packdataindex].rbuf[INDEX_DESTADDR];
	uint16_t srcaddress=rxqueue.rxData[packdataindex].rbuf[7]+(uint16_t)rxqueue.rxData[packdataindex].rbuf[8]*256;
	byte messagetype=rxqueue.rxData[packdataindex].rbuf[INDEX_MSGTYPE];
	byte sequenceid=rxqueue.rxData[packdataindex].rbuf[INDEX_SEQUENCE];
	//需要检查一下是否是本协议能处理的数据包，防止第三方数据包对系统造成破坏
	if (destaddress==0xffff)
	{
		if (rxqueue.rxData[packdataindex].rbuf[INDEX_MSGTYPE]!=MSGTYPE_NB && rxqueue.rxData[packdataindex].rbuf[INDEX_MSGTYPE]!=MSGTYPE_RB)
			return;
	}
	else
	{
		if (rxqueue.rxData[packdataindex].rbuf[INDEX_MSGTYPE]!=MSGTYPE_RD && rxqueue.rxData[packdataindex].rbuf[INDEX_MSGTYPE]!=MSGTYPE_RU && \
				rxqueue.rxData[packdataindex].rbuf[INDEX_MSGTYPE]!=MSGTYPE_DD && rxqueue.rxData[packdataindex].rbuf[INDEX_MSGTYPE]!=MSGTYPE_DU &&  rxqueue.rxData[packdataindex].rbuf[INDEX_MSGTYPE]!=MSGTYPE_RD_ACK )
			return;
		if (rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT]>MAX_ROUTER || rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT]<1) //hop limit
			return;
		if (rxqueue.rxData[packdataindex].rbuf[INDEX_DESTADDR]!=rxqueue.rxData[packdataindex].rbuf[INDEX_DESTADDR+rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT]+2] )  //dest address show in two different part
			return;
	}
	//+++++++ check finished
	byte pdataindex=INDEX_DESTADDR+1;
	byte pathindex=0;
	for ( nodeindex =1;nodeindex<=rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT];nodeindex++) //nb data is ok ,nodeindex =1
	{
		if (rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT+nodeindex]==localaddress || rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT+nodeindex]==BROADCASTADDR) //must have this address
			break;
	} //find my address location
	if (nodeindex>MAX_ROUTER) //error package
		return;
	//relay data p2p data
	_delay_ms(RELAYDELAY);//for  reduce comfilict
	if (destaddress_last!=localaddress && destaddress_last!=BROADCASTADDR ) //RB delay is later
	{

		MxRadio.beginTransmission(rxqueue.rxData[packdataindex].rbuf[nodeindex+1+INDEX_PATHCOUNT]);
		MxRadio.write((uint8_t *)(rxqueue.rxData[packdataindex].rbuf+INDEX_MSGTYPE),rxqueue.rxData[packdataindex].length-INDEX_MSGTYPE-2);
		MxRadio.endTransmission();
	}
	//relay finished
	//can't handle same seq message twice
	if (lasthandledseq[srcaddress_last]==sequenceid)
		return;
	else
		lasthandledseq[srcaddress_last]=sequenceid;

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
					neigbour[nbindex].rssi=rxqueue.rxData[packdataindex].rssi;
					neigbour[nbindex].intervalcountpast=0;
					break;
				}
			}
			if (lastrbseq!=rxqueue.rxData[packdataindex].rbuf[INDEX_SEQUENCE]) //handle before,every node broadcast one time
			{
				int mynodehop=rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT];
				lastrbseq=rxqueue.rxData[packdataindex].rbuf[INDEX_SEQUENCE];
				for ( nodeindex =1;nodeindex<=rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT];nodeindex++)
				{
					if (rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT+nodeindex]==BROADCASTADDR) //RB package finish by 255
						break;
					pathnode[mynodehop-nodeindex]=rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT+nodeindex];
				}

				pathnode[0]=mynodehop;
				pathnode[mynodehop]=SINKADDRESS;
				//first recieve this broadcat,need relay broadcast
				_delay_ms(1000);//for  reduce comfilict
				_delay_ms(4*localaddress);
				MxRadio.beginTransmission();
				rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT+rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT]]=srcaddress;
				rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT]++;
				rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT+rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT]]=BROADCASTADDR;
				MxRadio.write((uint8_t *)(rxqueue.rxData[packdataindex].rbuf+INDEX_MSGTYPE),rxqueue.rxData[packdataindex].length-INDEX_MSGTYPE-2+1);
				MxRadio.endTransmission();

				_delay_ms(RELAYDELAY);//upload neigbourinfo
				//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				//MSGTYPE SRCADDR DESTADDR SEQ PATHCOUNT NODE1 NODE2 NODE3 NEIGBOUR_NODEID RSSI ... CRC
				//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				if (localaddress!=0)
				{
					byte sendbuf[MAX_NB*2];
					byte sendbuflen;
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
			}
		}
		break;
	case MSGTYPE_RD: //download route ,every node in the path should handle,last node should ack
		pathcount=nodeindex;

		//record path
		for ( nodeindex =1;nodeindex<=rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT];nodeindex++)
		{
			pathnode[pathcount-nodeindex]=rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT+nodeindex];
			if (rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT+nodeindex]==localaddress)
				break;

		}
		pathnode[0]=pathcount;
		pathnode[pathcount]=SINKADDRESS;
		dataupload_interval=rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT+rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT]+1];
		if (dataupload_interval<10 && dataupload_interval>0)
			dataupload_interval=10;//interval must big than 10s
		if (rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT+rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT]]==localaddress) //find node should ack
		{
			//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			//MSGTYPE SRCADDR DESTADDR SEQ PATHCOUNT NODE1 NODE2 NODE3...  CRC
			//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			_delay_ms(RELAYDELAY);
			uploaddata(MSGTYPE_RD_ACK,NULL,0);
		}
		break;
	case MSGTYPE_DD:
		if (rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT]==nodeindex)//本节点是目标节点
		{
			rxqueue.rxData[packdataindex].payloadindex=INDEX_PATHCOUNT+nodeindex+1;
			serialpackage_send(rxqueue.rxData[packdataindex].rbuf,rxqueue.rxData[packdataindex].length);
		}
		break;
	case MSGTYPE_DU://data upload
		if (destaddress_last==SINKADDRESS)//本节点是目标节点
		{
			rxqueue.rxData[packdataindex].payloadindex=INDEX_PATHCOUNT+nodeindex+1;
			serialpackage_send(rxqueue.rxData[packdataindex].rbuf,rxqueue.rxData[packdataindex].length);
		}
		break;
	case MSGTYPE_RU: //data upload must forward to base 0

		if (rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT+rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT]]==SINKADDRESS)//本节点是目标节点
		{
			rxqueue.rxData[packdataindex].payloadindex=INDEX_PATHCOUNT+nodeindex+1;
			//Serial.write(rxqueue.rxData[packdataindex].rbuf,rxqueue.rxData[packdataindex].length);
			serialpackage_send(rxqueue.rxData[packdataindex].rbuf,rxqueue.rxData[packdataindex].length);
		}
		break;

	case MSGTYPE_NB:
		//just for get neigbour info
		int nbindex;
		for ( nbindex=0;nbindex<MAX_NB;nbindex++)
		{
			if (neigbour[nbindex].nodeid==BROADCASTADDR || neigbour[nbindex].nodeid==srcaddress)
			{
				neigbour[nbindex].nodeid=srcaddress;
				neigbour[nbindex].rssi=rxqueue.rxData[packdataindex].rssi;
				neigbour[nbindex].intervalcountpast=0;
				break;
			}
		}
		byte tmpbadrssi,tmptmpbadnodeidindex;
		if (nbindex==MAX_NB) //remove some worst rssi neigbour
		{
			for ( nbindex=0;nbindex<MAX_NB;nbindex++)
			{
				byte tmpbadrssi;
				if (tmpbadrssi<neigbour[nbindex].rssi)
				{
					tmpbadrssi=neigbour[nbindex].rssi;
					tmptmpbadnodeidindex=nbindex;
				}
			}
			if (tmpbadrssi>rxqueue.rxData[packdataindex].rssi+RSSI_STEP)
			{
				neigbour[tmptmpbadnodeidindex].nodeid=srcaddress;
				neigbour[tmptmpbadnodeidindex].rssi=rxqueue.rxData[packdataindex].rssi;
				neigbour[tmptmpbadnodeidindex].intervalcountpast=0;
			}
		}
		break;
	case MSGTYPE_RD_ACK:
		if (rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT+rxqueue.rxData[packdataindex].rbuf[INDEX_PATHCOUNT]]==SINKADDRESS)//本节点是目标节点
		{
			rxqueue.rxData[packdataindex].payloadindex=INDEX_PATHCOUNT+nodeindex+1;
			serialpackage_send(rxqueue.rxData[packdataindex].rbuf,rxqueue.rxData[packdataindex].length);
		}
		break;
	}
}
