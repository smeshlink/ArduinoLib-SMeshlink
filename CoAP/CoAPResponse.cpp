/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * CoAPResponse.cpp
 *
 *      Author: smeshlink
 */

#include <string.h>

#include "CoAPResponse.h"

CoAPResponse::CoAPResponse(void *response, uint8_t *buffer, int32_t *offset) {
	_response = response;
	_buffer = buffer;
	_payloadLength = 0;
	this->_offset = offset;
}

CoAPResponse::~CoAPResponse() {
}

size_t CoAPResponse::print(const char *s) {
	return write(s, strlen(s));
}

size_t CoAPResponse::printf(const char *format, ...) {
	va_list arg_ptr;
	va_start(arg_ptr, format);
	int printed = vsnprintf((char *)_buffer + _payloadLength, REST_MAX_CHUNK_SIZE - _payloadLength, format, arg_ptr);
	_payloadLength += printed;
	return printed;
}

size_t CoAPResponse::write(const char *buffer, size_t length) {
	size_t copied = REST_MAX_CHUNK_SIZE - _payloadLength;
	if (copied > length)
		copied = length;
	memcpy(_buffer + _payloadLength, buffer, copied);
	_payloadLength += copied;
	return copied;
}

void CoAPResponse::flush() {
	REST.set_response_payload(_response, _buffer, _payloadLength);
}

int CoAPResponse::setPath(const char *path) {
	return REST.set_url(_response, path);
}

int CoAPResponse::setContentType(unsigned int contentType) {
	return REST.set_header_content_type(_response, contentType);
}

int CoAPResponse::setStatusCode(unsigned int code) {
	return REST.set_response_status(_response, code);
}

int CoAPResponse::setPayload(const void *payload, size_t length) {
	return REST.set_response_payload(_response, payload, length);
}

void CoAPResponse::setPayloadLength(size_t length) {
	_payloadLength = length;
}

int CoAPResponse::setLocation(const char *location) {
	return REST.set_header_location(_response, location);
}

int CoAPResponse::setMaxAge(uint32_t age) {
	return REST.set_header_max_age(_response, age);
}

int CoAPResponse::setETag(const uint8_t *etag, size_t length) {
	return REST.set_header_etag(_response, etag, length);
}

int CoAPResponse::setBlock1(uint32_t num, uint8_t more, uint16_t size) {
	return coap_set_header_block1(_response, num, more, size);
}

int CoAPResponse::setBlock2(uint32_t num, uint8_t more, uint16_t size) {
	return coap_set_header_block2(_response, num, more, size);
}

int32_t CoAPResponse::getOffset() {
	return *_offset;
}

void CoAPResponse::blockAppend(int32_t length) {
	*_offset += length;
}

void CoAPResponse::blockComplete() {
	*_offset = -1;
}

CoAPObserveResponse::CoAPObserveResponse() : CoAPResponse(&notification, _buffer, NULL) {
	coap_init_message(&notification, COAP_TYPE_CON, CONTENT_2_05, 0);
}
