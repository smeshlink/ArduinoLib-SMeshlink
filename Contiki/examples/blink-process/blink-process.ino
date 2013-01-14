/*
 * Copyright (c) 2011-2013, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * blink-process.cpp
 *
 *      Author: smeshlink
 */

#include "contiki-arduino.h"
#include "Process.h"
#include "ExpiryTimer.h"

extern "C" {
#include "dev/leds.h"
}

#define BLINK 0
#define LED_BLINK 1

#if BLINK
class BlinkProcess : public Process
{
private:
	ExpiryTimer _timer;
	uint8_t _leds_state;
public:
	BlinkProcess() : _leds_state(0)
	{
	}
protected:
	virtual PT_THREAD(doRun())
	{
		/* Any process must start with this. */
		PROCESS_BEGIN();

		while(1) {
			/* Set the etimer every time. */
			_timer.start(1);

			/* And wait until the specific event. */
			PROCESS_WAIT_EVENT_UNTIL(_timer.expired());

			/* Change the state of leds. */
			leds_off(LEDS_ALL);
			leds_on(_leds_state);
			_leds_state += 1;
		}

		/* Any process must end with this, even if it is never reached. */
		PROCESS_END();
	}
} procBlink;
#endif

#if LED_BLINK
class LedBlinkProcess : public Process
{
private:
	ExpiryTimer _timer;
	int _seconds;
	uint8_t _led;
public:
	LedBlinkProcess(const char *name, uint8_t led, int seconds)
		: Process(name), _seconds(seconds), _led(led)
	{
	}
protected:
	virtual PT_THREAD(doRun())
	{
		/* Any process must start with this. */
		PROCESS_BEGIN();

		while(1) {
			/* Set the etimer every time. */
			_timer.start(_seconds);

			/* And wait until the specific event. */
			PROCESS_WAIT_EVENT_UNTIL(_timer.expired());

			/* Change the state of leds. */
			leds_toggle(_led);
		}

		/* Any process must end with this, even if it is never reached. */
		PROCESS_END();
	}
};
LedBlinkProcess procYellow("yellow blink", LEDS_YELLOW, 1);
LedBlinkProcess procGreen("green blink", LEDS_GREEN, 3);
#endif

void setup()
{
#if BLINK
	procBlink.run();
#endif
#if LED_BLINK
	procYellow.run();
	procGreen.run();
#endif
}
