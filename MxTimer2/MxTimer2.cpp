/*
  MxTimer2.h - Using timer2 with a configurable resolution user 32.768k
  Fredqian <fuqian@smeshlink.com>
  Changed from FlexTimer2

 */

#include <MxTimer2.h>

unsigned long MxTimer2::time_units;
void (*MxTimer2::func)();
volatile unsigned long MxTimer2::count;
volatile char MxTimer2::overflowing;



/**
 * @param resolution
 *  resolution mesh how many interrupt in 1 s
 */
void MxTimer2::set(unsigned long units, unsigned int resolution, void (*f)()) {


	if (units == 0)
		time_units = 1;
	else
		time_units = units;

	func = f;

#if defined (__AVR_ATmega1284P__) || (__AVR_AT90USB1287__) || (__AVR_ATmega1281__) || defined (__AVR_ATmega128RFA1__)
	ASSR = _BV(AS2);
	TCNT2 = 0;/* Set counter to zero */
	/*	Set comparison register: Crystal freq. is 32768, pre-scale factor is 8, we want CLOCK_CONF_SECOND ticks / sec:
	 * 32768 = 8 * CLOCK_CONF_SECOND * OCR2A, less 1 for CTC mode
	 */
	OCR2A = 32768/8/resolution - 1;
	/* 	 Set timer control register: - prescale: 8 (CS21) - counter reset via comparison register (WGM21)   */ 								\
	TCCR2A = _BV(WGM21);
	TCCR2B = _BV(CS21);
	/* Clear interrupt flag register */
	TIFR2 = TIFR2;
	/* Raise interrupt when value in OCR2 is reached. Note that the   * counter value in TCNT2 is cleared automatically.   */
	TIMSK2 &= ~_BV (OCIE2A);
#else
#error Unsupported CPU type
#endif


}

void MxTimer2::start() {
	count = 0;
	overflowing = 0;
#if defined (__AVR_ATmega1284P__) || (__AVR_AT90USB1287__) || (__AVR_ATmega1281__) || defined (__AVR_ATmega128RFA1__)

	TCNT2 = 0;
	TIMSK2 = _BV (OCIE2A);
#endif
}

void MxTimer2::stop() {
#if defined (__AVR_ATmega1284P__) || (__AVR_AT90USB1287__) || (__AVR_ATmega1281__) || defined (__AVR_ATmega128RFA1__)
	TIMSK2 &= ~_BV (OCIE2A);
#endif
}

void MxTimer2::_overflow() {
	count += 1;

	if (count >= time_units && !overflowing) {
		overflowing = 1;
		count = count - time_units;
		(*func)();
		overflowing = 0;
	}
}

ISR(TIMER2_COMPA_vect) {

	MxTimer2::_overflow();
}

