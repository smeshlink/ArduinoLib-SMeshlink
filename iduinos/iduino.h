
#ifndef __IDUINO_H__
#define __IDUINO_H__

#ifdef __cplusplus
extern "C" {
#endif

#define IDUINO 1

#include "stdint.h"

#ifndef IDUINO_VERSION_STRING
#define IDUINO_VERSION_STRING "iDuino 1.0"
#endif
#ifndef NULL
#define NULL 0
#endif /* NULL */


#define CCIF
#define CLIF


#define CLOCK_CONF_SECOND 1024
typedef unsigned long clock_time_t;
#define CLOCK_LT(a,b)  ((signed short)((a)-(b)) < 0)
#define INFINITE_TIME 0xffff
/* Pre-allocated memory for loadable modules heap space (in bytes)*/
#define MMEM_CONF_SIZE 256


#define CSN            0


#include "sys/process.h"
#include "sys/timer.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "sys/pt.h"
#include "sys/procinit.h"
#include "sys/clock.h"
#include "sys/list.h"
#include "sys/watchdog.h"
void setup();
void initialize();
void loop();

#ifdef __cplusplus
}
#endif

#endif /* __CONTIKI_H__ */
