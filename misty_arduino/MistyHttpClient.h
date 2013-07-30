
#ifndef MISTRYHTTPCLIENT_H
#define MISTRYHTTPCLIENT_H

#include <MistyFeed.h>
#include <psock.h>
#include "webclient.h"
#include <NanodeUIP.h>
#include <CountingStream.h>
#include <Print.h>

enum app_states_e
{
	state_none = 0, /**< No state has been set. Illegal */
	state_needip, /**< We are blocked until we get an IP address */
	state_needresolv, /**< We are blocked until we can resolve the host of our service */
	state_noconnection, /**< Everything is ready, webclient needs to be started */
	state_connecting, /**< Trying to connect to server */
	state_done, /**< Application complete.  Stopped. */
	state_invalid  /**< An invalid state.  Illegal */
};



class MistyHttpClient
{
public:
	static int get(MistyFeed& aFeed, const char* aApiKey);
	static int put(MistyFeed& aFeed, const char* aApiKey);
	MistyHttpClient();

protected:
	static const int kCalculateDataLength =0;
	static const int kSendData =1;
	static void buildPath(char* aDest, unsigned long aFeedId, const char* aFormat);


};

#endif
