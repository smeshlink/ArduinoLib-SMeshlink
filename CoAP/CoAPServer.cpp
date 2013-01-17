/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * CoAPServer.cpp
 *
 *      Author: smeshlink
 */

#include <string.h>

#include "contiki-arduino.h"
#include "CoAPServer.h"

CoAPServerImpl CoAPServer;

CoAPResource* CoAPServerImpl::list = NULL;

CoAPServerImpl::CoAPServerImpl() {
}

CoAPServerImpl::~CoAPServerImpl() {
}

void CoAPServerImpl::init() {
	initialize();
	rest_init_engine();
}

void CoAPServerImpl::addResource(CoAPResource* resource) {
	listAdd(resource);
	resource->activate(this);
}

void CoAPServerImpl::activate(CoAPResource* resource) {
	resource->_resource.handler = handleRequest;
	rest_activate_resource(&resource->_resource);
}

void CoAPServerImpl::activate(PeriodicResource* resource) {
	resource->_resource.handler = handleRequest;
	resource->_periodicResource.periodic_handler = handlePeriodic;
	rest_activate_periodic_resource(&resource->_periodicResource);
}

void CoAPServerImpl::activate(EventResource* resource) {
	resource->_resource.handler = handleRequest;
	rest_activate_event_resource(&resource->_resource);
}

void CoAPServerImpl::handleRequest(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
	CoAPRequest coapRequest(request, preferred_size);
	const char *url;
	int len = coapRequest.getPath(&url);
	CoAPResource *res = listFind(url, len);
	CoAPResponse coapResponse(response, buffer, offset);

	if (res != NULL) {
		res->handle(coapRequest, coapResponse);
		coapResponse.flush();
	}
	else
		REST.set_response_status(response, REST.status.NOT_FOUND);
}

void CoAPServerImpl::handlePeriodic(resource_t *resource) {
	PeriodicResource *res = (PeriodicResource *)listFind(resource->url, strlen(resource->url));
	if (res) {
		res->fire();
	}
}

void CoAPServerImpl::listAdd(CoAPResource *resource) {
	if (list == NULL) {
		list = resource;
	} else {
		CoAPResource *prev = list;
		while (prev->_next != NULL)
			prev = prev->_next;
		prev->_next = resource;
	}
}

CoAPResource* CoAPServerImpl::listFind(const char *url, int length) {
	CoAPResource *res = list;
	while (res != NULL) {
		if (strncmp(res->_resource.url, url, length) == 0)
			break;
		res = res->_next;
	}
	return res;
}
