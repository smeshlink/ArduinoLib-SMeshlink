/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * SeparateResource.h
 *
 *      Author: smeshlink
 */

#ifndef SEPARATERESOURCE_H_
#define SEPARATERESOURCE_H_

#include "SeparateResponse.h"

class SeparateResource : public CoAPResource {
private:
	struct ctimer _ctimer;
	SeparateResponse _separateResponse;
public:
	clock_time_t time;
public:
	SeparateResource(const char *url, clock_time_t time);
	virtual ~SeparateResource();
protected:
	virtual void doGet(CoAPRequest& request, CoAPResponse& response);
	virtual void begin(CoAPRequest& request) = 0;
	virtual void end(CoAPResponse& response) = 0;
private:
	static void ctimer_callback(void *ctx);
};

#endif /* SEPARATERESOURCE_H_ */
