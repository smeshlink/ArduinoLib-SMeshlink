#ifndef MxTimer2_h
#define MxTimer2_h

#ifdef __AVR__
#include <avr/interrupt.h>
#else
#error MxTimer2 library only works on AVR architecture
#endif


namespace MxTimer2 {
	extern unsigned long time_units;
	extern void (*func)();
	extern volatile unsigned long count;
	extern volatile char overflowing;
	extern volatile unsigned int tcnt2;
	
	void set(unsigned long ms, void (*f)());
	void set(unsigned long units, unsigned int resolution, void (*f)());
	void start();
	void stop();
	void _overflow();
}

#endif
