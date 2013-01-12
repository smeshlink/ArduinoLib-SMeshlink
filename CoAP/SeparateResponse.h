/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * SeparateResponse.h
 *
 *      Author: smeshlink
 */

#ifndef SEPARATERESPONSE_H_
#define SEPARATERESPONSE_H_

#include "CoAPServer.h"

extern "C" {
/* Required to manually (=not by the engine) handle the response transaction. */
#include "er-coap-07-separate.h"
#include "er-coap-07-transactions.h"
}

/* A structure to store the required information */
typedef struct separate_store {
  /* Provided by Erbium to store generic request information such as remote address and token. */
  coap_separate_t request_metadata;
  /* Add fields for addition information to be stored for finalizing, e.g.: */
  char buffer[64];
} separate_store_t;

class SeparateResponse : public CoAPResponse {
private:
	coap_packet_t _response;
	uint8_t _buffer[REST_MAX_CHUNK_SIZE];
	bool _active;
	separate_store_t _store;
public:
	SeparateResponse();
	~SeparateResponse();
	bool accept(CoAPRequest& request);
	bool send();
};

#endif /* SEPARATERESPONSE_H_ */
