#include <MistyHttpClient.h>


/**
 * The current state the application is in
 */
static app_states_e app_state = state_needip;


MistyHttpClient::MistyHttpClient()
{

}
int MistyHttpClient::put(MistyFeed& aFeed, const char* aApiKey)
{
  char path[30];
  buildPath(path, aFeed.id(), "json");

  int index = 0;
  char extra_headers[1024];
  index +=sprintf(extra_headers+index,"X-ApiKey: %s\n",aApiKey);
  index +=sprintf(extra_headers+index,"User-Agent: Misty-Arduino-Lib/1.0",aApiKey);
  //index +=sprintf(extra_headers+index,"Host","api.xively.com");
  CountingStream countingStream;
  countingStream.httpBufIndex=0;
  int bodylen=(&countingStream)->print(aFeed);

  webclient_init();
  webclient_put("api.xively.com", 	\
		  80, 						 \
		  path, 						\
		  (const char*)extra_headers, (const char*)countingStream.httpBuf,bodylen );

  return 1;
}

void MistyHttpClient::buildPath(char* aDest, unsigned long aFeedId, const char* aFormat)
{
  char idstr[12]; 
  strcpy(aDest, "/v2/feeds/");
  char* p = &idstr[10];
  idstr[11] = 0;
  for(*p--=aFeedId%10+0x30;aFeedId/=10;*p--=aFeedId%10+0x30);
  strcat(aDest, p+1);
  strcat(aDest, ".");
  strcat(aDest, aFormat);
}

int MistyHttpClient::get(MistyFeed& aFeed, const char* aApiKey)
{

  char path[30];
  buildPath(path, aFeed.id(), "csv");

  int index = 0;
  char extra_headers[1024];
  index +=sprintf(extra_headers+index,"X-ApiKey: %s\n",aApiKey);
  index +=sprintf(extra_headers+index,"User-Agent: Misty-Arduino-Lib/1.0",aApiKey);
  //index +=sprintf(extra_headers+index,"Host","api.xively.com");

  webclient_init();
  webclient_getwithheader("api.xively.com", 80, path, (const char*)extra_headers);
  return 1;
}

// Stats
uint32_t size_received = 0;
uint32_t started_at = 0;

/****************************************************************************/
/**
 * Callback function that is called from the webclient code when HTTP
 * data has been received.
 *
 * This function must be implemented by the module that uses the
 * webclient code. The function is called from the webclient module
 * when HTTP data has been received. The function is not called when
 * HTTP headers are received, only for the actual data.
 *
 * \note This function is called many times, repetedly, when data is
 * being received, and not once when all data has been received.
 *
 * \param data A pointer to the data that has been received.
 * \param len The length of the data that has been received.
 */
void webclient_datahandler(char *data, u16_t len)
{
	//printf_P(PSTR("%lu: webclient_datahandler data=%p len=%u\r\n"),millis(),data,len);
	Serial.print('.');

	if ( ! started_at )
		started_at = millis();

	size_received += len;
#if 1
	// Dump out the text
	while(len--)
	{
		char c = *data++;
		if ( c == '\n' )
			Serial.print('\r');
		Serial.print(c);
	}
	Serial.println();
#endif

	// If data is NULL, we are done.  Print the stats
	if (!data)
	{
		Serial.println();
		printf_P(PSTR("%lu: DONE. Received %lu bytes in %lu msec.\r\n"),millis(),size_received,millis()-started_at);
		app_state = state_done;
		started_at = 0;
	}
}

/****************************************************************************/
/**
 * Callback function that is called from the webclient code when the
 * HTTP connection has been connected to the web server.
 *
 * This function must be implemented by the module that uses the
 * webclient code.
 */
void webclient_connected(void)
{
	uip_log_P(PSTR("webclient_connected"));
}

/****************************************************************************/
/**
 * Callback function that is called from the webclient code if the
 * HTTP connection to the web server has timed out.
 *
 * This function must be implemented by the module that uses the
 * webclient code.
 */
void webclient_timedout(void)
{
	printf_P(PSTR("webclient_timedout\r\n"));
	app_state = state_noconnection;
}

/****************************************************************************/
/**
 * Callback function that is called from the webclient code if the
 * HTTP connection to the web server has been aborted by the web
 * server.
 *
 * This function must be implemented by the module that uses the
 * webclient code.
 */
void webclient_aborted(void)
{
	printf_P(PSTR("webclient_aborted\r\n"));
	app_state = state_noconnection;
}

/****************************************************************************/
/**
 * Callback function that is called from the webclient code when the
 * HTTP connection to the web server has been closed.
 *
 * This function must be implemented by the module that uses the
 * webclient code.
 */
void webclient_closed(void)
{
	printf_P(PSTR("webclient_closed\r\n"));
	app_state = state_noconnection;
}


