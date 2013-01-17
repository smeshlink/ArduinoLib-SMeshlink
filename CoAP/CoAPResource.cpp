/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * CoAPResource.cpp
 *
 *      Author: smeshlink
 */

#include "CoAPResource.h"

CoAPResource::CoAPResource(const char *url, const char *attributes, CoAPHandler GET, CoAPHandler POST,
		CoAPHandler PUT, CoAPHandler DELETE) : _next(NULL), GET(GET), POST(POST), PUT(PUT), DELETE(DELETE) {
	_resource.next = NULL;
	_resource.flags = (rest_resource_flags_t)(METHOD_GET | METHOD_POST | METHOD_PUT | METHOD_DELETE);
	_resource.url = url;
	_resource.attributes = attributes;
	_resource.handler = NULL;
	_resource.pre_handler = NULL;
	_resource.post_handler = NULL;
	_resource.user_data = NULL;
}

CoAPResource::~CoAPResource() {
}

void CoAPResource::setAttributes(const char *attributes) {
	_resource.attributes = attributes;
}

void CoAPResource::activate(CoAPServerImpl *server) {
	server->activate(this);
}

void CoAPResource::handle(CoAPRequest& request, CoAPResponse& response) {
	switch (request.getMethod()) {
	case METHOD_GET:
		doGet(request, response);
		break;
	case METHOD_POST:
		doPost(request, response);
		break;
	case METHOD_PUT:
		doPut(request, response);
		break;
	case METHOD_DELETE:
		doDelete(request, response);
		break;
	default:
		break;
	}
}

void CoAPResource::doGet(CoAPRequest& request, CoAPResponse& response) {
	if (GET)
		GET(request, response);
	else
		response.setStatusCode(REST.status.METHOD_NOT_ALLOWED);
}

void CoAPResource::doPost(CoAPRequest& request,CoAPResponse& response) {
	if (POST)
		POST(request, response);
	else
		response.setStatusCode(REST.status.METHOD_NOT_ALLOWED);
}

void CoAPResource::doPut(CoAPRequest& request, CoAPResponse& response) {
	if (PUT)
		PUT(request, response);
	else
		response.setStatusCode(REST.status.METHOD_NOT_ALLOWED);
}

void CoAPResource::doDelete(CoAPRequest& request, CoAPResponse& response) {
	if (DELETE)
		DELETE(request, response);
	else
		response.setStatusCode(REST.status.METHOD_NOT_ALLOWED);
}

EventResource::EventResource(const char *url, const char *attributes, CoAPHandler GET,
		CoAPHandler POST, CoAPHandler PUT, CoAPHandler DELETE)
		: CoAPResource(url, attributes, GET, POST, PUT, DELETE), obsCounter(0) {
}

void EventResource::activate(CoAPServerImpl *server) {
	server->activate(this);
}

bool EventResource::trigger(CoAPResponse& response) {
	/*
	 * Perform GET by default, override this method to behavior different,
	 * and return false if DO NOT want to trigger this time.
	 * */
	CoAPRequest emptyRequest(NULL, 0);
	doGet(emptyRequest, response);
	return true;
}

void EventResource::fire() {
	CoAPObserveResponse response;
	obsCounter++;
	if (trigger(response)) {
		response.flush();
		REST.notify_subscribers(&_resource, obsCounter, &response.notification);
	}
}

PeriodicResource::PeriodicResource(const char *url, uint32_t period, const char *attributes,
		CoAPHandler GET, CoAPHandler POST, CoAPHandler PUT, CoAPHandler DELETE)
		: EventResource(url, attributes, GET, POST, PUT, DELETE) {
	_periodicResource.next = NULL;
	_periodicResource.periodic_timer.next = NULL;
	_periodicResource.periodic_timer.p = NULL;
	_periodicResource.periodic_timer.timer.interval = 0;
	_periodicResource.periodic_timer.timer.start = 0;
	setPeriod(period);
}

void PeriodicResource::setPeriod(uint32_t period) {
	_period = period;
	if (period > 0) {
		_periodicResource.period = period;
		_periodicResource.resource = &_resource;
	}
}

void PeriodicResource::activate(CoAPServerImpl *server) {
	server->activate(this);
}
