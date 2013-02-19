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




/* This function deals with all incoming events for the "hello
   world" application, including dealing with new connections. */
static void hello_world_appcall(void)
{
	uint8_t sendbuf[100];
	uint8_t buflen;
	buflen=0;
	char *hello="Hi,This is a TCP test!\n";
	if(uip_connected())
	{
		uip_send(hello,strlen(hello)+1);
	}

	if(conn->tcpstateflags == UIP_ESTABLISHED   )
	{
		while (Serial.available())
		{
			sendbuf[buflen]=Serial.read();
			buflen++;
		}
		if (buflen)
		{
			uip_send(sendbuf,buflen);
		}
	}
	if(uip_newdata())
	{
		//((char *)uip_appdata)[uip_len]=0;
		Serial.write((uint8_t *)uip_appdata,uip_len);
	}
}

void setup() {
	pinMode(20,OUTPUT);
	digitalWrite(20,LOW);
	char buf[20];
	byte macaddr[6] = { 0x2, 0x00, 0x00, 0x1, 0x2, 0x3 };
	uip.init(macaddr,34);
	Serial.begin(38400);
	uip.get_mac_str(buf);
	//Serial.println(buf);
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
