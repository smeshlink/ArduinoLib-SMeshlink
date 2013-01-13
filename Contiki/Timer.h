/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * Timer.h
 *
 *      Author: smeshlink
 */

#ifndef TIMER_H_
#define TIMER_H_

extern "C" {
#include "contiki.h"
}

class Timer {
private:
	struct ctimer _ctimer;
public:
	clock_time_t time;
	bool autoReset;
	void *context;
	void (*tick)(void *ctx);
public:
	Timer(clock_time_t time, bool autoReset = false, void (*tick)(void *ctx) = NULL, void *context = NULL);
	virtual ~Timer();
	void start();
	void stop();
private:
	static void ctimer_callback(void *ctx);
};

#endif /* TIMER_H_ */
