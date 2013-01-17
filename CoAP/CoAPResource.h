/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * CoAPResource.h
 *
 *      Author: smeshlink
 */

#ifndef COAPRESOURCE_H_
#define COAPRESOURCE_H_

class CoAPResource;
class EventResource;
class PeriodicResource;

#include "CoAPServer.h"

typedef void(*CoAPHandler)(CoAPRequest&, CoAPResponse&);

class CoAPResource {
private:
	CoAPResource *_next;
	void handle(CoAPRequest& request, CoAPResponse& response);
	friend class CoAPServerImpl;
protected:
	resource_t _resource;
public:
	CoAPHandler GET;
	CoAPHandler POST;
	CoAPHandler PUT;
	CoAPHandler DELETE;
public:
	CoAPResource(const char *url, const char *attributes = NULL, CoAPHandler GET = NULL,
			CoAPHandler POST = NULL, CoAPHandler PUT = NULL, CoAPHandler DELETE = NULL);
	virtual ~CoAPResource();
	void setAttributes(const char *attributes);
protected:
	virtual void activate(CoAPServerImpl *server);
	virtual void doGet(CoAPRequest& request, CoAPResponse& response);
	virtual void doPost(CoAPRequest& request, CoAPResponse& response);
	virtual void doPut(CoAPRequest& request, CoAPResponse& response);
	virtual void doDelete(CoAPRequest& request, CoAPResponse& response);
};

class EventResource : public CoAPResource {
public:
	uint16_t obsCounter;
	EventResource(const char *url, const char *attributes = NULL, CoAPHandler GET = NULL,
			CoAPHandler POST = NULL, CoAPHandler PUT = NULL, CoAPHandler DELETE = NULL);
protected:
	virtual void activate(CoAPServerImpl *server);
	virtual bool trigger(CoAPResponse& response);
public:
	virtual void fire();
};

class PeriodicResource : public EventResource {
private:
	uint32_t _period;
	periodic_resource_t _periodicResource;
	friend class CoAPServerImpl;
public:
	PeriodicResource(const char *url, uint32_t period, const char *attributes = NULL, CoAPHandler GET = NULL,
			CoAPHandler POST = NULL, CoAPHandler PUT = NULL, CoAPHandler DELETE = NULL);
	void setPeriod(uint32_t period);
protected:
	virtual void activate(CoAPServerImpl *server);
};

#endif /* COAPRESOURCE_H_ */
