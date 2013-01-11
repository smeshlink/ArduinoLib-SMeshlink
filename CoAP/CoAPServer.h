/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * CoAPServer.h
 *
 *      Author: smeshlink
 */

#ifndef COAPSERVER_H_
#define COAPSERVER_H_

extern "C" {
#include "erbium.h"
}

class CoAPServerImpl;

extern CoAPServerImpl CoAPServer;

#include "CoAPRequest.h"
#include "CoAPResponse.h"
#include "CoAPResource.h"

class CoAPServerImpl {
private:
	static CoAPResource *list;
	static void handleRequest(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
	static void handlePeriodic(resource_t *resource);
	static void listAdd(CoAPResource *resource);
	static CoAPResource* listFind(const char *url, int length);
public:
	CoAPServerImpl();
	virtual ~CoAPServerImpl();
	void init();
	void addResource(CoAPResource *resource);
	void activate(CoAPResource* resource);
	void activate(PeriodicResource* resource);
	void activate(EventResource* resource);
};

#endif /* COAPSERVER_H_ */
