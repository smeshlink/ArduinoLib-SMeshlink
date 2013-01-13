/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * blink-timer.cpp
 *
 *      Author: smeshlink
 */

#include "contiki-arduino.h"
#include "Timer.h"

extern "C" {
#include "dev/leds.h"
}

void blinkTimerTick(void *ctx)
{
	uint8_t *state = (uint8_t *)ctx;
	leds_off(LEDS_ALL);
	leds_on(*state);
	*state += 1;
}

static uint8_t leds_state = 0;
Timer blinkTimer(CLOCK_SECOND * 2, true, blinkTimerTick, &leds_state);

void setup()
{
	blinkTimer.start();
}
