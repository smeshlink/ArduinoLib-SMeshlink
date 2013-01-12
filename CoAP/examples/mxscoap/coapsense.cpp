extern "C" {
#include "contiki.h"
#include "net/tcpip.h"
#include "dev/leds.h"
#include "contiki-net.h"
#include "erbium.h"
#include "dev/leds.h"
#include "er-coap-07.h"
#include "sys/energest.h"
//const struct rest_implementation REST;

}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Arduino.h>
#include "MXSConfig.h"

#include "CoAPServer.h"

#define RES_MXS1101 0
#define RES_MXS1201 0
#define RES_MXS1202 0
#define RES_MXS1301 0
#define RES_MXS1401 0
#define RES_MXS1501 0
#define RES_MXS2101 0
#define RES_MXS4101 1
#define REST_RES_SEPARATE 1

//#define Serial1 Serial
//The setup function is called once at startup of the sketch
MXSConfig myMXSConfig = MXSConfig();

struct CoapARG
{
	void* request;
	void* response;
	uint8_t *buffer;
	uint16_t preferred_size;
	int32_t *offset;
} myARG;
struct ctimer c;
/*---------------------------------------------------------------------------*/
PROCESS(coap_sample, "Camera CoAP Server");
//AUTOSTART_PROCESSES(&coap_sample);

#if RES_MXS2101
#include "MXS2101.h"

static MXS2101 myMXS2101;
static uint8_t blocking = 0;

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
			if (blocking)
		      return;
		    blocking = 1;

		    if (request.getVariable("size", &temp)) {
    			camera_size = myMXS2101.camera_parse_size(temp[0]);
		    }

			//delay(2000);
			leds_toggle(LEDS_GREEN);
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
			blocking = 0;
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

		leds_toggle(LEDS_GREEN);
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
		float tempc ,humidity ;
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
MXS1301 myMXS1301=MXS1301();
static void
my_timer_callback(void *in)
{
	char temp[32] = "";
	uint16_t myvalue = myMXS1301.getCO2();
	sprintf(temp, "%d", myvalue);
	REST.set_header_content_type(myARG.response, TEXT_PLAIN);
	REST.set_response_payload(myARG.response, temp, strlen(temp));
	myMXS1301.StopSensor();
}
/*
 * Get MXS1301 Value
 */
RESOURCE(MXS1301, METHOD_GET, "MXS1301", "MXS1301");
void
MXS1301_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

	myMXS1301.StartSensor();
	//delay(60000);
	///////////////////////////////////////////////////// need test
	static struct etimer mytimer;
	etimer_set(&mytimer, CLOCK_CONF_SECOND*60);
	while(!etimer_expired(&mytimer));
	/////////////////////////////////////////////////////need test
	myARG.buffer=buffer;
	myARG.request=request;
	myARG.response=response;
	myARG.preferred_size=preferred_size;
	myARG.offset=offset;
	ctimer_set(&c, 60*CLOCK_CONF_SECOND,my_timer_callback, (void *)NULL);
	/*char temp[32] = "";
	uint16_t myvalue=myMXS1301.getCO2();
	sprintf(temp, "%d", myvalue);
	REST.set_header_content_type(response, TEXT_PLAIN);
	REST.set_response_payload(response, temp, strlen(temp));

	myMXS1301.StopSensor();
	*/
}
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



#if REST_RES_SEPARATE


extern "C" {
/* Required to manually (=not by the engine) handle the response transaction. */
#include "er-coap-07-separate.h"
#include "er-coap-07-transactions.h"
}

/* A structure to store the required information */
typedef struct application_separate_store {
  /* Provided by Erbium to store generic request information such as remote address and token. */
  coap_separate_t request_metadata;
  /* Add fields for addition information to be stored for finalizing, e.g.: */
  char buffer[64];
} application_separate_store_t;

class SeparateResource : public CoAPResource {
private:
	struct ctimer _ctimer;
	bool _active;
	application_separate_store_t _store[1];
public:
	clock_time_t time;
public:
	SeparateResource(const char *url, clock_time_t time)
		: CoAPResource(url), _active(false), time(time) {
	}
	virtual ~SeparateResource() {
		ctimer_stop(&_ctimer);
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		if (_active)
			coap_separate_reject();
		else {
			_active = true;
			/* Take over and skip response by engine. */
			coap_separate_accept(request.request, &_store->request_metadata);
			/* Be aware to respect the Block2 option, which is also stored in the coap_separate_t. */

			leds_on(LEDS_YELLOW);
			leds_toggle(LEDS_GREEN);
			begin(request);
			ctimer_set(&_ctimer, time, ctimer_callback, this);
		}
	}
	virtual void begin(CoAPRequest& request) = 0;
	virtual void end(CoAPResponse& response) = 0;
private:
	void timeout() {
		coap_transaction_t *transaction = NULL;
		if ( (transaction = coap_new_transaction(_store->request_metadata.mid, &_store->request_metadata.addr, _store->request_metadata.port)) )
		    {
		  coap_packet_t response[1]; /* This way the packet can be treated as pointer as usual. */

		  /* Restore the request information for the response. */
		        coap_separate_resume(response, &_store->request_metadata, CONTENT_2_05);

//		  uint8_t buffer[REST_MAX_CHUNK_SIZE];
//		  CoAPResponse coapResponse(response, buffer, NULL);
//
//		  /*
//		   * Be aware to respect the Block2 option, which is also stored in the coap_separate_t.
//		   * As it is a critical option, this example resource pretends to handle it for compliance.
//		   */
//		  coapResponse.setBlock2(_store.request_metadata.block2_num, 0, _store.request_metadata.block2_size);
//
//		  end(coapResponse);


			REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
			coap_set_payload(response, "test", 4);

			coap_set_header_block2(response, _store->request_metadata.block2_num, 0, _store->request_metadata.block2_size);

			      /* Warning: No check for serialization error. */
			      transaction->packet_len = coap_serialize_message(response, transaction->packet);
			      coap_send_transaction(transaction);
			      /* The engine will clear the transaction (right after send for NON, after acked for CON). */

			leds_off(LEDS_YELLOW);

		  /* Warning: No check for serialization error. */
		  transaction->packet_len = coap_serialize_message(response, transaction->packet);
		  coap_send_transaction(transaction);
		  /* The engine will clear the transaction (right after send for NON, after acked for CON). */

		  _active = false;
		} else {
			leds_toggle(LEDS_RED);
		  //PRINTF("ERROR (transaction)\n");
		}
	}
	static void ctimer_callback(void *ctx) {
		SeparateResource *res = (SeparateResource *)ctx;
//		ctimer_reset(&res->_ctimer);
		res->timeout();
		leds_toggle(LEDS_GREEN);
	}
};

class HelloResource : public SeparateResource {
private:
	int urlLen;
	const char* url;
public:
	HelloResource() : SeparateResource("hello", 3 * CLOCK_SECOND)
	{
	}
protected:
	virtual void begin(CoAPRequest& request)
	{
		urlLen = request.getPath(&url);
	}
	virtual void end(CoAPResponse& response)
	{
		response.setContentType(REST.type.TEXT_PLAIN);
		response.printf("delayed hello from %s", url);
	}
} resHello;

/* Required to manually (=not by the engine) handle the response transaction. */
#include "er-coap-07-separate.h"
#include "er-coap-07-transactions.h"
/*
 * Resource which cannot be served immediately and which cannot be acknowledged in a piggy-backed way
 */
PERIODIC_RESOURCE(separate, METHOD_GET, "separate", "title=\"r\"", 3*CLOCK_SECOND);

#define PRINTF(...)
static uint8_t separate_active = 0;
static application_separate_store_t separate_store[1];

void
separate_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_packet_t *const coap_req = (coap_packet_t *) request;

  PRINTF("/separate       ");
  if (separate_active)
  {
    PRINTF("REJECTED ");
    coap_separate_reject();
  }
  else
  {
    PRINTF("STORED ");
    separate_active = 1;

    /* Take over and skip response by engine. */
    coap_separate_accept(request, &separate_store->request_metadata);
    /* Be aware to respect the Block2 option, which is also stored in the coap_separate_t. */

    snprintf(separate_store->buffer, 64, "Type: %u\nCode: %u\nMID: %u", coap_req->type, coap_req->code, coap_req->mid);
  }

  PRINTF("(%s %u)\n", coap_req->type==COAP_TYPE_CON?"CON":"NON", coap_req->mid);
}

void
separate_periodic_handler(resource_t *resource)
{
  if (separate_active)
  {
    PRINTF("/separate       ");
    coap_transaction_t *transaction = NULL;
    if ( (transaction = coap_new_transaction(separate_store->request_metadata.mid, &separate_store->request_metadata.addr, separate_store->request_metadata.port)) )
    {
      PRINTF("RESPONSE (%s %u)\n", separate_store->request_metadata.type==COAP_TYPE_CON?"CON":"NON", separate_store->request_metadata.mid);

      coap_packet_t response[1]; /* This way the packet can be treated as pointer as usual. */

      /* Restore the request information for the response. */
      coap_separate_resume(response, &separate_store->request_metadata, CONTENT_2_05);

      REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
      coap_set_payload(response, separate_store->buffer, strlen(separate_store->buffer));

      /*
       * Be aware to respect the Block2 option, which is also stored in the coap_separate_t.
       * As it is a critical option, this example resource pretends to handle it for compliance.
       */
      coap_set_header_block2(response, separate_store->request_metadata.block2_num, 0, separate_store->request_metadata.block2_size);

      /* Warning: No check for serialization error. */
      transaction->packet_len = coap_serialize_message(response, transaction->packet);
      coap_send_transaction(transaction);
      /* The engine will clear the transaction (right after send for NON, after acked for CON). */

      separate_active = 0;
    } else {
      PRINTF("ERROR (transaction)\n");
    }
  } /* if (separate_active) */
}
#endif



class TimerResource : public EventResource {
private:
	struct ctimer _ctimer;
public:
	clock_time_t interval;
	bool autoReset;
public:
	TimerResource(const char *url, clock_time_t interval, bool autoReset)
		: EventResource(url), interval(interval), autoReset(autoReset) {
	}
	void start() {
		ctimer_set(&_ctimer, CLOCK_SECOND, ctimer_callback, this);
	}
	void stop() {
		ctimer_stop(&_ctimer);
	}
protected:
	virtual void timeout() {
		fire();
	}
private:
	static void ctimer_callback(void *ctx) {
		TimerResource *res = (TimerResource *)ctx;
		res->timeout();
		if (res->autoReset)
			ctimer_reset(&res->_ctimer);
	}
};

class CounterResource : public TimerResource {
private:
	struct ctimer _ctimer;
public:
	CounterResource() : TimerResource("counter", CLOCK_SECOND, true)
	{
	}
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response)
	{
		start();
		response.setContentType(REST.type.TEXT_PLAIN);
		response.printf("%d", obsCounter);
	}
	virtual bool trigger(CoAPResponse& response) {
		leds_toggle(LEDS_YELLOW);
		return true;
	}
} resCounter;


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coap_sample, ev, data)
{
	static struct etimer timer;

	/* Any process must start with this. */
	PROCESS_BEGIN();

	/* Set the etimer to generate an event in one second. */
	etimer_set(&timer, CLOCK_CONF_SECOND);

	while(1) {
		/* Wait for an event. */
		PROCESS_WAIT_EVENT();

		/* Got the timer's event~ */
		if (ev == PROCESS_EVENT_TIMER) {
			resCounter.fire();

			/* Reset the etimer so it will generate another event after the exact same time. */
			etimer_reset(&timer);
		}
	} // while (1)

	/*
	uint8_t mySensortype=myMXSConfig.GetSensorType();
	if (mySensortype!=0)
	{
		if (mySensortype==C_MXS1101)
			rest_activate_resource(&resource_MXS1101);
		else if (mySensortype==C_MXS1201)
			rest_activate_resource(&resource_MXS1201);
		else if (mySensortype==C_MXS1301)
			rest_activate_resource(&resource_MXS1301);
		else if (mySensortype==C_MXS1401)
			rest_activate_resource(&resource_MXS1401);
		else if (mySensortype==C_MXS1501)
			rest_activate_resource(&resource_MXS1501);
		else if (mySensortype==C_MXS1202)
			rest_activate_resource(&resource_MXS1202);
		else if (mySensortype==C_MXS4101)
			{
			rest_activate_resource(&resource_MXS4101);
			rest_activate_resource(&resource_MXS4101_periodic);
			}
		else if (mySensortype==C_MXS2101)
		{
			rest_activate_resource(&resource_camera);
			rest_activate_resource(&resource_take);
			rest_activate_resource(&resource_size);
			rest_activate_resource(&resource_count);
			rest_activate_resource(&resource_packet);
		}
	}
*/
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/


#include "contiki-arduino.h"

void config()
{
	arduino_set_node_id(0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 0x34);
//	arduino_set_channel(26);
//	arduino_set_power(0);
}

void setup()
{
	CoAPServer.init();

	  rest_activate_periodic_resource(&periodic_resource_separate);

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
#if RES_MXS4101
	CoAPServer.addResource(&resMXS4101);
#endif
	CoAPServer.addResource(&resHello);
	CoAPServer.addResource(&resCounter);
#if RES_MXS2101
	myMXS2101.StartSensor();
	CoAPServer.addResource(&resMXS2101);
	CoAPServer.addResource(&resTake);
	CoAPServer.addResource(&resCount);
	CoAPServer.addResource(&resSize);
	CoAPServer.addResource(&resPacket);
#endif
	//resCounter.start();
}
