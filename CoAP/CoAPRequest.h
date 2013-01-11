/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * CoAPRequest.h
 *
 *      Author: smeshlink
 */

#ifndef COAPREQUEST_H_
#define COAPREQUEST_H_

extern "C" {
#include "erbium.h"

#ifndef WITH_COAP
#define WITH_COAP 7
#endif

/* For CoAP-specific example: not required for normal RESTful Web service. */
#if WITH_COAP == 3
#include "er-coap-03.h"
#define CONTENT_2_05 OK_200
#elif WITH_COAP == 7
#include "er-coap-07.h"
#else
#warning "Erbium example without CoAP-specifc functionality"
#endif /* CoAP-specific example */
}

class CoAPRequest {
public:
	void* _request;
	const uint16_t preferredSize;
public:
	CoAPRequest(void* request, uint16_t preferredSize);
	virtual ~CoAPRequest();
	int getPath(const char **url);
	rest_resource_flags_t getMethod();
	uint8_t getType();
	uint8_t getCode();
	uint8_t getId();
	int getQuery(const char **value);
	unsigned int getContentType();
	int getPayload(uint8_t **payload);
	int getHost(const char **host);
	int getAccept(const uint16_t **accept);
	int getQueryVariable(const char *name, const char **value);
	int getPostVariable(const char *name, const char **value);
	int getVariable(const char *name, const char **value);
	uint32_t getBlock1Num();
	uint16_t getBlock1Size();
};

#endif /* COAPREQUEST_H_ */
