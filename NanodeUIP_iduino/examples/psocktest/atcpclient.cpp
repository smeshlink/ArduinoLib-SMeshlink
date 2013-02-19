// Do not remove the include below
#include "atcpclient.h"
#include <psock.h>
#include <NanodeUIP.h>
uip_ipaddr_t tcpserver;
struct hello_world_state {
	struct psock p;
	byte inputbuffer[100];
	char name[20];
	char quest[20];
};
UIPASSERT(sizeof(struct hello_world_state)<=TCP_APP_STATE_SIZE)
struct uip_conn *conn;
static int hello_world_connection2(struct hello_world_state *s)
{
	PSOCK_BEGIN(&s->p);

	PSOCK_SEND_STR(&s->p, "Hello. What is your name?\n>>> ");
	PSOCK_READTO(&s->p, '\n');
	/* The input buffer is an array of bytes received from the
     network.  It's not a string.  We cheekily cast it to a string
     here on the assumption that we're receiving plain ASCII and
     not UTF8 or anything more fancy... */
	s->inputbuffer[PSOCK_DATALEN(&s->p)]=0;
	strncpy(s->name, (const char *)s->inputbuffer, sizeof(s->name));
	PSOCK_SEND_STR(&s->p, "What is your quest?\n>>> ");
	PSOCK_READTO(&s->p, '\n');
	s->inputbuffer[PSOCK_DATALEN(&s->p)]=0;
	strncpy(s->quest, (const char *)s->inputbuffer, sizeof(s->quest));
	PSOCK_SEND_STR(&s->p, "Hello ");
	PSOCK_SEND_STR(&s->p, s->name);
	PSOCK_SEND_STR(&s->p, "Your quest is: ");
	PSOCK_SEND_STR(&s->p, s->quest);
	PSOCK_CLOSE(&s->p);

	PSOCK_END(&s->p);
}
static
PT_THREAD(senddata(struct hello_world_state *s))
{
	PSOCK_BEGIN(&s->p);


	uint8_t sendbuf[100];
	uint8_t buflen;
	buflen=0;

	while (Serial.available())
	{
		sendbuf[buflen]=Serial.read();
		Serial.write(sendbuf[buflen]);
		buflen++;
	}
	if (buflen)
	{
		PSOCK_SEND(&s->p,sendbuf,buflen);
		//uip_flags=uip_flags&(~UIP_NEWDATA);
	}

	PSOCK_END(&s->p);
}

/* This function defines what the "hello world" application does
   once a connection has been established.  It doesn't have to
   deal with setting up the connection. */
static int hello_world_connection(struct hello_world_state *s)
{
	PSOCK_BEGIN(&s->p);

	while (	conn->tcpstateflags ==UIP_ESTABLISHED)
	{

		//Serial.write("OK");

		/*
		uint8_t sendbuf[100];
		uint8_t buflen;
		buflen=0;

		while (Serial.available())
		{
			sendbuf[buflen]=Serial.read();
			Serial.write(sendbuf[buflen]);
			buflen++;
		}
		if (buflen)
		{
			PSOCK_SEND(&s->p,sendbuf,buflen);
			//uip_flags=uip_flags&(~UIP_NEWDATA);
		}
		*/
		PSOCK_WAIT_THREAD(&s->p,senddata(s));
		PSOCK_WAIT_UNTIL(&s->p,PSOCK_NEWDATA(&s->p));
		{
			PSOCK_READBUF_LEN(&s->p,1);
			s->inputbuffer[PSOCK_DATALEN(&s->p)]=0;
			Serial.write((const char *)s->inputbuffer);
		}

	}

	PSOCK_CLOSE(&s->p);

	PSOCK_END(&s->p);
}

/* This function deals with all incoming events for the "hello
   world" application, including dealing with new connections. */
static void hello_world_appcall(void)
{
	struct hello_world_state *s = (struct hello_world_state *)&(uip_conn->appstate);

	/*
	 * If a new connection was just established, we should initialize
	 * the protosocket in our application's state structure.
	 */
	if(uip_connected()) {
		PSOCK_INIT(&s->p, s->inputbuffer, sizeof(s->inputbuffer)-1);
	}

	/*
	 * We run the protosocket function that actually handles the
	 * communication. We pass it a pointer to the application state
	 * of the current connection.
	 */
	hello_world_connection(s);
}

void setup() {
	pinMode(20,OUTPUT);
	digitalWrite(20,LOW);
	char buf[20];
	byte macaddr[6] = { 0x2, 0x00, 0x00, 0x1, 0x2, 0x3 };
	uip.init(macaddr,34);
	Serial.begin(38400);
	uip.get_mac_str(buf);
	Serial.println(buf);
	uip.wait_for_link();
	uip.set_ip_addr(192,168,1,15);
	//uip.start_dhcp(dhcp_status);
	uip_ipaddr(tcpserver,192, 168, 1, 100);
	//udp_init(&udpserver);
	conn=uip_connect(&tcpserver, UIP_HTONS(80), hello_world_appcall);

}



void loop() {
	uip.poll();
	//delay(5000);
	//s.state = STATE_SENDING;
	//udp_init(&udpserver);

	if(conn->tcpstateflags == UIP_CLOSED   )
	{
		//Serial.write("error");
		conn=uip_connect(&tcpserver, UIP_HTONS(80), hello_world_appcall);
	}

}

/*---------------------------------------------------------------------------*/
