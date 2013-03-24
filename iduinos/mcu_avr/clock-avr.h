#ifndef CONTIKI_CLOCK_AVR_H
#define CONTIKI_CLOCK_AVR_H


#if defined (__AVR_ATmega1284P__) || (__AVR_AT90USB1287__) || (__AVR_ATmega1281__) || defined (__AVR_ATmega128RFA1__)

#if AVR_CONF_USE32KCRYSTAL || 1
#define AVR_OUTPUT_COMPARE_INT TIMER2_COMPA_vect
#define OCRSetup() \
  TCCR2A = _BV(WGM21); \
  /* Clock from crystal on TOSC0-1 */ \
  ASSR = _BV(AS2);		      \
\
  /* Set counter to zero */   \
  TCNT2 = 0;				  \
\
  /*						  \
   * Set comparison register: \
   * Crystal freq. is 32768,\
   * pre-scale factor is 8, we want CLOCK_CONF_SECOND ticks / sec: \
   * 32768 = 8 * CLOCK_CONF_SECOND * OCR2A, less 1 for CTC mode\
   */ \
  OCR2A = 32768/8/CLOCK_CONF_SECOND - 1; \
\
  /* 								\
   * Set timer control register: 	\
   *  - prescale: 8 (CS21) \
   *  - counter reset via comparison register (WGM21) \
   */ 								\
  TCCR2A = _BV(WGM21); \
  TCCR2B = _BV(CS21);  \
\
  /* Clear interrupt flag register */ \
  TIFR2 = TIFR2; \
\
  /* \
   * Raise interrupt when value in OCR2 is reached. Note that the \
   * counter value in TCNT2 is cleared automatically. \
   */ \
  TIMSK2 = _BV (OCIE2A);
#endif /* !AVR_CONF_USE32KCRYSTAL */



#else
#error "Setup CPU in clock-avr.h"
#endif

#endif //CONTIKI_CLOCK_AVR_H
