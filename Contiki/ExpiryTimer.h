/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * ExpiryTimer.h
 *
 *      Author: smeshlink
 */

#ifndef EXPIRYTIMER_H_
#define EXPIRYTIMER_H_

extern "C" {
#include "contiki.h"
}

class ExpiryTimer {
private:
	struct etimer _timer;
public:
	void start(float interval) {
		etimer_set(&_timer, CLOCK_SECOND * interval);
	}
	bool expired() {
		return etimer_expired(&_timer);
	}
};

#endif /* EXPIRYTIMER_H_ */
