// Do not remove the include below
#include "atcpclient.h"
#include <NanodeUIP.h>
#include <pt.h>
#include <uip.h>
#define STATE_INITIAL         0
#define STATE_SENDING         1
#define STATE_RECEIVED        2
struct udp_state {
	struct pt pt;
	char state;
	struct uip_udp_conn *conn;
	struct timer timer;
	u16_t ticks;

	uint32_t unixtime;  // Seconds since Jan 1, 1970
};
static struct udp_state s;
#define SERVER_PORT  801
#define CLIENT_PORT  900
const char *sendstr="hello world";
uip_ipaddr_t udpserver;
void handledata()
{
	char *str2;
	str2=(char *)uip_appdata;
	//for (int i=0;i<UIP_IPTCPH_LEN + UIP_LLH_LEN;i++)
	Serial.print(str2);
}
static
PT_THREAD(handle_udp(void))
{
	PT_BEGIN(&s.pt);

	/* try_again:*/
	s.state = STATE_SENDING;
	s.ticks = CLOCK_SECOND;

	do {
		uip_send((const void *)sendstr, strlen(sendstr));

		/* Sending does not clear the NEWDATA flag.  The packet doesn't
       actually get sent until we yield at least once.  If we don't
       clear the flag ourselves, we will enter an infinite loop here.
       This is arguably a bug in uip.c and the uip_send() function
       should probably clear the NEWDATA flag. */
		uip_flags=uip_flags&(~UIP_NEWDATA);
		timer_set(&s.timer, s.ticks);
		PT_WAIT_UNTIL(&s.pt, uip_newdata() || timer_expired(&s.timer));

		if(uip_newdata() ) {
			handledata();

			//s.state = STATE_RECEIVED;
			//break;
		}

		if(s.ticks < CLOCK_SECOND * 2) {
			s.ticks *= 2;
		}
	} while(s.state != STATE_RECEIVED);

	s.ticks = CLOCK_SECOND;



	/*  timer_stop(&s.timer);*/

	/*
	 * PT_END restarts the thread so we do this instead. Eventually we
	 * should reacquire expired leases here.
	 */
	while(1) {
		PT_YIELD(&s.pt);
	}

	PT_END(&s.pt);
}

/*---------------------------------------------------------------------------*/
void
udp_appcall(void)
{
	handle_udp();
}


/*---------------------------------------------------------------------------*/
int
udp_init(const uip_ipaddr_t* addr)
{

	s.state = STATE_INITIAL;
	//s.conn = uip_udp_new(const_cast<uip_ipaddr_t*>(addr), UIP_HTONS(SERVER_PORT),udp_appcall);
	if(s.conn != NULL) {

		PT_INIT(&s.pt);
		return 1;
	}
	return 0;
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
	uip_ipaddr(udpserver,192, 168, 1, 100);
	//udp_init(&udpserver);
	s.conn = uip_udp_new(0, 0,udp_appcall);
	uip_udp_bind(s.conn, UIP_HTONS(CLIENT_PORT));
}



void loop() {
	uip.poll();
	//delay(5000);
	//s.state = STATE_SENDING;
	//udp_init(&udpserver);

}

/*---------------------------------------------------------------------------*/
