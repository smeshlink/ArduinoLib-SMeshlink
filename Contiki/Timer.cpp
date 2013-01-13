/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * Timer.cpp
 *
 *      Author: smeshlink
 */

#include "Timer.h"
#include "contiki-arduino.h"

Timer::Timer(clock_time_t time, bool autoReset, void (*tick)(void *ctx), void *context)
	: time(time), autoReset(autoReset), context(context), tick(tick) {
}

Timer::~Timer() {
	ctimer_stop(&_ctimer);
}

void Timer::start() {
	initialize();
	ctimer_set(&_ctimer, time, ctimer_callback, this);
}

void Timer::stop() {
	ctimer_stop(&_ctimer);
}

void Timer::ctimer_callback(void *ctx) {
	Timer *timer = (Timer *)ctx;
	timer->tick(timer->context);
	if (timer->autoReset)
		ctimer_reset(&timer->_ctimer);
}
