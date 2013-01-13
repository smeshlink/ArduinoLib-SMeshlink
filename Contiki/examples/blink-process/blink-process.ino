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

extern "C" {
#include "dev/leds.h"
}

#define BLINK 0
#define LED_BLINK 1

#if BLINK
class BlinkProcess : public Process
{
private:
	struct etimer _timer;
	uint8_t _leds_state;
public:
	BlinkProcess() : _leds_state(0)
	{
	}
protected:
	virtual PT_THREAD(thread(struct pt *process_pt, process_event_t ev, process_data_t data))
	{
		/* Any process must start with this. */
		PROCESS_BEGIN();

		while(1) {
			/* Set the etimer every time. */
			etimer_set(&_timer, CLOCK_SECOND * 1);
			/* And wait until the specific event. */
			PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

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
	struct etimer _timer;
	clock_time_t _time;
	uint8_t _led;
public:
	LedBlinkProcess(const char *name, uint8_t led, clock_time_t time) : Process(name), _time(time), _led(led)
	{
	}
protected:
	virtual PT_THREAD(thread(struct pt *process_pt, process_event_t ev, process_data_t data))
	{
		/* Any process must start with this. */
		PROCESS_BEGIN();

		while(1) {
			/* Set the etimer every time. */
			etimer_set(&_timer, _time);
			/* And wait until the specific event. */
			PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

			/* Change the state of leds. */
			leds_toggle(_led);
		}

		/* Any process must end with this, even if it is never reached. */
		PROCESS_END();
	}
};
LedBlinkProcess procYellow("yellow blink", LEDS_YELLOW, CLOCK_SECOND * 1);
LedBlinkProcess procGreen("green blink", LEDS_GREEN, CLOCK_SECOND * 3);
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
