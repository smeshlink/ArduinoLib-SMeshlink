/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * coap-sensors.cpp
 *
 *      Author: smeshlink
 */

#include "CoAPServer.h"

#define RES_MXS1101 0
#define RES_MXS1201 0
#define RES_MXS1202 0
#define RES_MXS1301 1
#define RES_MXS1401 1
#define RES_MXS1501 1
#define RES_MXS2101 0
#define RES_MXS4101 1

#if RES_MXS2101
#include "MXS2101.h"

static MXS2101 myMXS2101;

class CameraResource : public CoAPResource
{
public:
	CameraResource() : CoAPResource("camera")
	{
		setAttributes("title=\"camera\"");
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		takePicture(request, response);
	}
	virtual void doPost(CoAPRequest& request, CoAPResponse& response)
	{
		takePicture(request, response);
	}
private:
	void takePicture(CoAPRequest& request, CoAPResponse& response)
	{
		const char *temp;
		uint8_t buff[128];
		camera_size_t camera_size = CAMERA_SIZE_1;
		uint16_t count;
		uint32_t size;
		uint32_t index = 0, len = 0, tries = 10;
		int32_t offset = response.getOffset();

		if (0 == offset)
		{
		    if (request.getVariable("size", &temp)) {
    			camera_size = myMXS2101.camera_parse_size(temp[0]);
		    }

			myMXS2101.camera_take_picture(camera_size, request.preferredSize, &size, &count);
		}

		index = offset / request.preferredSize + 1;
		count = myMXS2101.camera_get_packet_count();
		len = myMXS2101.camera_try_get_packet(index, buff, tries);

		response.setContentType(REST.type.IMAGE_JPEG);
		response.write((char *)buff, len);
		response.blockAppend(len);

		if (index >= count) {
			response.blockComplete();
			//myMXS2101.StopSensor();
		}
	}
} resMXS2101;

class TakeResource : public CoAPResource
{
public:
	TakeResource() : CoAPResource("take")
	{
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		takePicture(request, response);
	}
	virtual void doPost(CoAPRequest& request, CoAPResponse& response)
	{
		takePicture(request, response);
	}
private:
	void takePicture(CoAPRequest& request, CoAPResponse& response)
	{
		const char *temp;
		camera_size_t camera_size = CAMERA_SIZE_1;
		uint16_t count;
		uint32_t size;

		if (request.getVariable("size", &temp)) {
			camera_size = myMXS2101.camera_parse_size(temp[0]);
		}

		myMXS2101.camera_take_picture(camera_size, request.preferredSize, &size, &count);

		response.setContentType(TEXT_PLAIN);
		response.printf("{'t':'OK','s':%lu,'n':%u}", size, count);
	}
} resTake;

class SizeResource : public CoAPResource
{
public:
	SizeResource() : CoAPResource("size")
	{
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		response.setContentType(TEXT_PLAIN);
		response.printf("%lu", myMXS2101.camera_get_picture_size());
	}
} resSize;

class CountResource : public CoAPResource
{
public:
	CountResource() : CoAPResource("count")
	{
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		response.setContentType(TEXT_PLAIN);
		response.printf("%u", myMXS2101.camera_get_packet_count());
	}
} resCount;

class PacketResource : public CoAPResource
{
public:
	PacketResource() : CoAPResource("packet")
	{
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		getPacket(request, response);
	}
	virtual void doPost(CoAPRequest& request, CoAPResponse& response)
	{
		getPacket(request, response);
	}
	virtual void doPut(CoAPRequest& request, CoAPResponse& response)
	{
		getPacket(request, response);
	}
private:
	void getPacket(CoAPRequest& request, CoAPResponse& response)
	{
		const char *temp;
		char buf[128];
		int no = 0, len = 0, tries = 0;

		/* Get the index of the specified packet. */
		if (request.getVariable("no", &temp))
		{
			memcpy(buf, temp, len);
			buf[len] = '\0';
			no = atoi(buf);
		}

		/* Get count of tries. Unused right now. */
		if (request.getVariable("try", &temp))
		{
			memcpy(buf, temp, len);
			buf[len] = '\0';
			tries = atoi(buf);
		}

		if (tries <= 0) {
			tries = 10;
		}

		len = myMXS2101.camera_try_get_packet(no, (uint8_t*)buf, tries);
		response.setContentType(TEXT_PLAIN);
		response.write(buf, len);
	}
} resPacket;
#endif

#if RES_MXS1101
#include "MXS1101.h"

class MXS1101Resource : public PeriodicResource
{
private:
	MXS1101 myMXS1101;
public:
	MXS1101Resource() : PeriodicResource("MXS1101", CLOCK_SECOND), myMXS1101()
	{
		setAttributes("title=\"MXS1101 temperature\"");
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		myMXS1101.StartSensor();
		float tempc = myMXS1101.getTempC();
		myMXS1101.StopSensor();

		response.setContentType(REST.type.TEXT_PLAIN);
		response.printf("%d.%d", (int)tempc, (int)(tempc * 100) - ((int)tempc) * 100);
	}
} resMXS1101;
#endif

#if RES_MXS1201
#include "MXS1201.h"

class MXS1201Resource : public CoAPResource
{
private:
	MXS1201 myMXS1201;
public:
	MXS1201Resource() : CoAPResource("MXS1201"), myMXS1201()
	{
		setAttributes("title=\"MXS1201 temperature and humidity\"");
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		myMXS1201.StartSensor();
		float tempc = myMXS1201.getTempC();
		float humidity = myMXS1201.getHumidity();
		myMXS1201.StopSensor();

		response.setContentType(REST.type.TEXT_PLAIN);
		response.printf("temp %d.%d, humi %d.%d", (int)tempc, (int)(tempc * 100) - ((int)tempc) * 100,
				(int)humidity, (int)(humidity * 100) - ((int)humidity) * 100);
	}
} resMXS1201;
#endif

#if RES_MXS1202
#include "MXS1202.h"

class MXS1202Resource : public CoAPResource
{
private:
	MXS1202 myMXS1202;
public:
	MXS1202Resource() : CoAPResource("MXS1202"), myMXS1202()
	{
		setAttributes("title=\"MXS1202 temperature and humidity\"");
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		float tempc, humidity;
		myMXS1202.StartSensor();
		int result = myMXS1202.getTempCandHumidity(&tempc, &humidity);
		result = myMXS1202.getTempCandHumidity(&tempc, &humidity);
		myMXS1202.StopSensor();

		response.setContentType(REST.type.TEXT_PLAIN);
		response.printf("temp %d.%d, humi %d.%d", (int)tempc, (int)(tempc * 100) - ((int)tempc) * 100,
				(int)humidity, (int)(humidity * 100) - ((int)humidity) * 100);
	}
} resMXS1202;
#endif

#if RES_MXS1301
#include "MXS1301.h"
#include "SeparateResource.h"

class MXS1301Resource : public SeparateResource {
private:
	MXS1301 myMXS1301;;
public:
	MXS1301Resource() : SeparateResource("MXS1301", 3 * CLOCK_SECOND), myMXS1301()
	{
		setAttributes("title=\"MXS1301 co2\"");
	}
protected:
	virtual void begin(CoAPRequest& request)
	{
		myMXS1301.StartSensor();
	}
	virtual void end(CoAPResponse& response)
	{
		uint16_t myvalue = myMXS1301.getCO2();
		myMXS1301.StopSensor();
		response.setContentType(REST.type.TEXT_PLAIN);
		response.printf("%d", myvalue);
	}
} resMXS1301;
#endif

#if RES_MXS1401
#include "MXS1401.h"

class MXS1401Resource : public CoAPResource
{
private:
	MXS1401 myMXS1401;
public:
	MXS1401Resource() : CoAPResource("MXS1401"), myMXS1401()
	{
		setAttributes("title=\"MXS1401 soil humidity\"");
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		myMXS1401.StartSensor();
		float myvalue = myMXS1401.getSoilHumdity();
		myMXS1401.StopSensor();

		response.setContentType(REST.type.TEXT_PLAIN);
		response.printf("%d.%d", (int)myvalue, (int)(myvalue * 100) - ((int)myvalue) * 100);
	}
} resMXS1401;
#endif

#if RES_MXS1501
#include "MXS1501.h"

class MXS1501Resource : public CoAPResource
{
private:
	MXS1501 myMXS1501;
public:
	MXS1501Resource() : CoAPResource("MXS1501"), myMXS1501()
	{
		setAttributes("title=\"MXS1501 light\"");
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		myMXS1501.StartSensor();
		float myvalue = myMXS1501.getLight();
		myMXS1501.StopSensor();

		response.setContentType(REST.type.TEXT_PLAIN);
		response.printf("%d.%d", (int)myvalue, (int)(myvalue * 100) - ((int)myvalue) * 100);
	}
} resMXS1501;
#endif

#if RES_MXS4101
#include "MXS4101.h"

class MXS4101Resource : public PeriodicResource
{
private:
	MXS4101 myMXS4101;
public:
	MXS4101Resource() : PeriodicResource("MXS4101", CLOCK_SECOND / 32)
	{
		setAttributes("title=\"MXS4101 proximity\"");
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		uint8_t result = myMXS4101.getValue();
		response.setContentType(REST.type.TEXT_PLAIN);
		response.printf("%d", result);
	}
#if 1
	/*
	 * Perform GET by default, override this method to behavior different,
	 * and return false if DO NOT want to trigger this time.
	 * */
	virtual bool trigger(CoAPResponse& response)
	{
		uint8_t result = myMXS4101.getValue();
		if (result == 0)
		{
			response.setContentType(REST.type.TEXT_PLAIN);
			response.printf("%d", result);
			return true;
		}
		else
			return false;
	}
#endif
} resMXS4101;
#endif

#include "contiki-arduino.h"

void setup()
{
	arduino_set_node_id(0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 0x34);
//	arduino_set_channel(26);
//	arduino_set_power(0);

	CoAPServer.init();

#if RES_MXS1101
	CoAPServer.addResource(&resMXS1101);
#endif
#if RES_MXS1201
	CoAPServer.addResource(&resMXS1201);
#endif
#if RES_MXS1202
	CoAPServer.addResource(&resMXS1202);
#endif
#if RES_MXS1401
	CoAPServer.addResource(&resMXS1401);
#endif
#if RES_MXS1501
	CoAPServer.addResource(&resMXS1501);
#endif
#if RES_MXS1301
	CoAPServer.addResource(&resMXS1301);
#endif
#if RES_MXS4101
	CoAPServer.addResource(&resMXS4101);
#endif
#if RES_MXS2101
	myMXS2101.StartSensor();
	CoAPServer.addResource(&resMXS2101);
	CoAPServer.addResource(&resTake);
	CoAPServer.addResource(&resCount);
	CoAPServer.addResource(&resSize);
	CoAPServer.addResource(&resPacket);
#endif
}
