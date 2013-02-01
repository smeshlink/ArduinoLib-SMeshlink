#include "iduino.h"
#include "Arduino.h"
#include "Process.h"
#include "ExpiryTimer.h"


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
		pinMode(27,OUTPUT);
		while(1) {
			/* Set the etimer every time. */
			_timer.start(1);

			/* And wait until the specific event. */
			PROCESS_WAIT_EVENT_UNTIL(_timer.expired());

			/* Change the state of leds. */
			digitalWrite(27,!digitalRead(27));
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
	LedBlinkProcess(const char *name, uint8_t ledpin, int seconds)
		: Process(name), _seconds(seconds), _led(ledpin)
	{
	}
protected:
	virtual PT_THREAD(doRun())
	{
		/* Any process must start with this. */
		PROCESS_BEGIN();
		pinMode(_led,OUTPUT);
		while(1) {
			/* Set the etimer every time. */
			_timer.start(_seconds);

			/* And wait until the specific event. */
			PROCESS_WAIT_EVENT_UNTIL(_timer.expired());

			/* Change the state of leds. */
			digitalWrite(_led,!digitalRead(_led));
		}

		/* Any process must end with this, even if it is never reached. */
		PROCESS_END();
	}
};
LedBlinkProcess procYellow("yellow blink", 29, 1);
LedBlinkProcess procGreen("green blink", 28, 3);
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
void loop()
{}
