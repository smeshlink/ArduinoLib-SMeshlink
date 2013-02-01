/*
 * Copyright (c) 2012, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */
/**
 *  \brief This module contains AVR-specific code to implement
 *  the Contiki core clock functions.
 *
 *  \author David Kopf <dak664@embarqmail.com> and others.
 *
 */
/** \addtogroup avr
 * @{
 */
/**
 *  \defgroup avrclock AVR clock implementation
 * @{
 */
/**
 *  \file
 *  This file contains AVR-specific code to implement the Contiki core clock functions.
 *
 */
/**
 * These routines define the AVR-specific calls declared in /core/sys/clock.h
 * CLOCK_SECOND is the number of ticks per second.
 * It is defined through CONF_CLOCK_SECOND in the contiki-conf.h for each platform.
 * The usual AVR defaults are 128 or 125 ticks per second, counting a prescaled CPU clock
 * using the 8 bit timer0.
 *
 * clock_time_t is usually declared by the platform as an unsigned 16 bit data type,
 * thus intervals up to 512 or 524 seconds can be measured with ~8 millisecond precision.
 * For longer intervals the 32 bit clock_seconds() is available.
 *
 * Since a carry to a higer byte can occur during an interrupt, declaring them non-static
 * for direct examination can cause occasional time reversals!
 *
 * clock-avr.h contains the specific setup code for each mcu.
 */
#include "sys/clock.h"

#include "clock-avr.h"
#include "sys/etimer.h"

#include <avr/io.h>
#include <avr/interrupt.h>


static volatile clock_time_t count;
volatile unsigned long milliseconds;
volatile unsigned long seconds;
/* sleepseconds is the number of seconds sleeping since startup, available globally */
long sleepseconds;


/*---------------------------------------------------------------------------*/
/**
 * Start the clock by enabling the timer comparison interrupts. 
 */
void
clock_init(void)
{
	cli ();
	OCRSetup();
	sei ();
}
/*---------------------------------------------------------------------------*/
/**
 * Return the tick counter. When 16 bit it typically wraps every 10 minutes.
 * The comparison avoids the need to disable clock interrupts for an atomic
 * read of the multi-byte variable.
 */
clock_time_t
clock_time(void)
{
	clock_time_t tmp;
	do {
		tmp = count;
	} while(tmp != count);
	return tmp;
}
/*---------------------------------------------------------------------------*/
/**
 * Return seconds, default is time since startup.
 * The comparison avoids the need to disable clock interrupts for an atomic
 * read of the four-byte variable.
 */
unsigned long
clock_seconds(void)
{
	unsigned long tmp;
	do {
		tmp = seconds;
	} while(tmp != seconds);
	return tmp;
}
/*---------------------------------------------------------------------------*/
/**
 * Set seconds, e.g. to a standard epoch for an absolute date/time.
 */
void
clock_set_seconds(unsigned long sec)
{
	seconds = sec;
}
/*---------------------------------------------------------------------------*/
/**
 * Return seconds, default is time since startup.
 * The comparison avoids the need to disable clock interrupts for an atomic
 * read of the four-byte variable.
 */
unsigned long
clock_milliseconds(void)
{
	unsigned long tmp;
	do {
		tmp = milliseconds;
	} while(tmp != milliseconds);
	return tmp;
}
/*---------------------------------------------------------------------------*/
/**
 * Set seconds, e.g. to a standard epoch for an absolute date/time.
 */
void
clock_set_milliseconds(unsigned long msec)
{
	milliseconds = msec;
}
/*---------------------------------------------------------------------------*/
/**
 * Wait for a number of clock ticks.
 */
void
clock_wait(clock_time_t t)
{
	clock_time_t endticks = clock_time() + t;
	if (sizeof(clock_time_t) == 1) {
		while ((signed char )(clock_time() - endticks) < 0) {;}
	} else if (sizeof(clock_time_t) == 2) {
		while ((signed short)(clock_time() - endticks) < 0) {;}
	} else {
		while ((signed long )(clock_time() - endticks) < 0) {;}
	}
}
void
clock_adjust_ticks(unsigned long howmany)
{
	uint8_t sreg = SREG;cli();
	count  += howmany;
	milliseconds=howmany*1024 / CLOCK_SECOND+milliseconds;
	while(howmany >= CLOCK_SECOND) {
		howmany -= CLOCK_SECOND;
		seconds++;

	}

	SREG=sreg;
}

/*---------------------------------------------------------------------------*/
void
clock_arch_sleepms(unsigned char howlong)
{
	/* Deep Sleep for howlong rtimer ticks. This will stop all timers except
	 * for TIMER2 which can be clocked using an external crystal.
	 * Unfortunately this is an 8 bit timer; a lower prescaler gives higher
	 * precision but smaller maximum sleep time.
	 * Here a maximum 128msec (contikimac 8Hz channel check sleep) is assumed.
	 * The rtimer and system clocks are adjusted to reflect the sleep time.
	 */
#include <avr/sleep.h>
#include <sys/watchdog.h>


	/* Save TIMER2 configuration for clock.c is using it */
	uint8_t savedTCNT2=TCNT2, savedTCCR2A=TCCR2A, savedTCCR2B = TCCR2B, savedOCR2A = OCR2A;

	cli();
	watchdog_stop();
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);

	/* Set TIMER2 clock asynchronus from external source, CTC mode */
	ASSR |= (1 << AS2);
	TCCR2A =(1<<WGM21);
	// Prescale by  32 - 1024 ticks/sec, 250 msec max sleep
	TCCR2B =((0<<CS22)|(1<<CS21)|(1<<CS20));
	OCR2A = howlong;

	/* Reset timer count, wait for the write (which assures TCCR2x and OCR2A are finished) */
	TCNT2 = 0;
	while(ASSR & (1 << TCN2UB));

	/* Enable TIMER2 output compare interrupt, sleep mode and sleep */
	TIMSK2 |= (1 << OCIE2A);
	SMCR |= (1 <<  SE);
	sei();
	/* Ensure no activity is suspending the low power mode
	   before going to sleep. (edited by smeshlink) */
	if (OCR2A && (get_mcu_need_alive_state()==0)) sleep_mode();
	//...zzZZZzz...Ding!//

	/* Disable sleep mode after wakeup, so random code cant trigger sleep */
	SMCR  &= ~(1 << SE);





	/* Restore clock.c configuration */
	cli();
	TCCR2A = savedTCCR2A;
	TCCR2B = savedTCCR2B;
	OCR2A  = savedOCR2A;
	TCNT2  = savedTCNT2;
	sei();
	clock_adjust_ticks(howlong);
	watchdog_start();



}
static uint8_t mcuneedalive = 0;

uint8_t
get_mcu_need_alive_state()
{
	return mcuneedalive;
}

void
set_mcu_need_alive_state(uint8_t value)
{
	cli();
	mcuneedalive=value;
	sei();
}

void
push_mcu_need_alive_state()
{
	cli();
	mcuneedalive++;
	sei();
}

void
pop_mcu_need_alive_state()
{
	cli();
	mcuneedalive--;
	sei();
}

ISR(TIMER2_COMPA_vect)
{
	count++;
	milliseconds=1024 / CLOCK_SECOND+milliseconds;
	if(count%CLOCK_SECOND==0) {
		seconds++;

	}

	/*  gcc will save all registers on the stack if an external routine is called */
	if(etimer_pending()) {
		etimer_request_poll();
	}

}

/*---------------------------------------------------------------------------*/


