/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * CoAPResponse.h
 *
 *      Author: smeshlink
 */

#ifndef COAPRESPONSE_H_
#define COAPRESPONSE_H_

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

class CoAPResponse {
private:
	void *_response;
	uint8_t *_buffer;
	size_t _payloadLength;
	int32_t *_offset;
public:
	CoAPResponse(void *response, uint8_t *buffer, int32_t *offset);
	virtual ~CoAPResponse();
	size_t write(const char *buffer, size_t length);
	size_t print(const char *s);
	size_t printf(const char *format, ...);
	void flush();
	int setPath(const char *path);
	int setContentType(unsigned int contentType);
	int setStatusCode(unsigned int code);
	int setPayload(const void *payload, size_t length);
	void setPayloadLength(size_t length);
	int setLocation(const char *location);
	int setMaxAge(uint32_t age);
	int setETag(const uint8_t *etag, size_t length);
	int setBlock1(uint32_t num, uint8_t more, uint16_t size);
	int32_t getOffset();
	void blockAppend(int32_t length);
	void blockComplete();
};

class CoAPObserveResponse : public CoAPResponse {
private:
	uint8_t _buffer[REST_MAX_CHUNK_SIZE];
public:
	coap_packet_t notification;
public:
	CoAPObserveResponse();
};

#endif /* COAPRESPONSE_H_ */
