/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * SeparateResource.cpp
 *
 *      Author: smeshlink
 */

#include "SeparateResource.h"

SeparateResource::SeparateResource(const char *url, clock_time_t time)
	: CoAPResource(url), time(time) {
}

SeparateResource::~SeparateResource() {
	ctimer_stop(&_ctimer);
}

void SeparateResource::doGet(CoAPRequest& request, CoAPResponse& response) {
	if (_separateResponse.accept(request)) {
		begin(request);
		ctimer_set(&_ctimer, time, ctimer_callback, this);
	}
}

void SeparateResource::ctimer_callback(void *ctx) {
	SeparateResource *res = (SeparateResource *)ctx;
	res->end(res->_separateResponse);
	res->_separateResponse.send();
}
