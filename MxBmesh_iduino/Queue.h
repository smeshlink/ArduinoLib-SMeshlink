/*
 *  QUEUE.h
 *
 *  Created on: 2013-5-9
 *      Author: fu
 */

#ifndef QUEUE_H_
#define  QUEUE_H_
#include <Arduino.h>
#define PACKAGE_MAX		128
#if (RAMEND < 16000)
#define	RFQUENEMAX 		18
#else
#define	RFQUENEMAX 		40
#endif

struct RfData
{
	byte rbuf[PACKAGE_MAX];
	byte length;
	byte payloadindex;
	union
	{
		byte rssi;
		byte destaddress;
	} value;
};

class  QUEUE {
private:

	byte front;
	byte rear;;
public:
	struct RfData RfData[RFQUENEMAX];
	QUEUE();
	void init_queue();
	byte inqueue( );
	byte dequeue();
	void undodequeue();
	byte peerqueue();

};

#endif /* QUEUE_H_ */
