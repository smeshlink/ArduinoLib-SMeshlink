/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * CoAPRequest.cpp
 *
 *      Author: smeshlink
 */

#include "CoAPRequest.h"

CoAPRequest::CoAPRequest(void* request, uint16_t preferredSize)
	: preferredSize(preferredSize) {
	this->request = request;
}

CoAPRequest::~CoAPRequest() {
}

int CoAPRequest::getPath(const char **url) {
	return REST.get_url(request, url);
}

rest_resource_flags_t CoAPRequest::getMethod() {
	return REST.get_method_type(request);
}

uint8_t CoAPRequest::getType() {
	return ((coap_packet_t *) request)->type;
}

uint8_t CoAPRequest::getCode() {
	return ((coap_packet_t *) request)->code;
}

uint8_t CoAPRequest::getId() {
	return ((coap_packet_t *) request)->mid;
}

uint32_t CoAPRequest::getBlock1Num() {
	return ((coap_packet_t *) request)->block1_num;
}

uint16_t CoAPRequest::getBlock1Size() {
	return ((coap_packet_t *) request)->block1_size;
}

int CoAPRequest::getQuery(const char **value) {
	return REST.get_query(request, value);
}

unsigned int CoAPRequest::getContentType() {
	return REST.get_header_content_type(request);
}

int CoAPRequest::getPayload(uint8_t **payload) {
	return REST.get_request_payload(request, payload);
}

int CoAPRequest::getHost(const char **host) {
	return REST.get_header_host(request, host);
}

int CoAPRequest::getAccept(const uint16_t **accept) {
	return REST.get_header_accept(request, accept);
}

int CoAPRequest::getQueryVariable(const char *name, const char **value) {
	return REST.get_query_variable(request, name, value);
}

int CoAPRequest::getPostVariable(const char *name, const char **value) {
	return REST.get_post_variable(request, name, value);
}

int CoAPRequest::getVariable(const char *name, const char **value) {
	return getQueryVariable(name, value) || getPostVariable(name, value);
}
