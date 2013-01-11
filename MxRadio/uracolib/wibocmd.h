/* Copyright (c) 2010 Axel Wachtler, Daniel Thiele
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of the authors nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE. */

/* $Id$ */

#ifndef WIBOCMD_H_
#define WIBOCMD_H_

#include <stdint.h>

typedef enum {
	WIBOSTATUS_IDLE = 0x00,
	WIBOSTATUS_RECEIVINGDATA = 0x01,
	WIBOSTATUS_ERROR = 0xFF
} bootlstatus_t;

typedef enum {
	WIBOERROR_NONE = 0x00,
	WIBOERROR_DATAMISS,
	WIBOERROR_SUCCESS
} bootlerrorcode_t;

typedef enum {
	WIBOCMDCODE_NONE = 0x00, 	/**< no command */
	WIBOCMDCODE_PING, 			/**< Ping a node which replies and delivers information about itself */
	WIBOCMDCODE_PINGREPLY, 		/**< Reply to a ping request */
	WIBOCMDCODE_DATA, 			/**< Feed a node with data */
	WIBOCMDCODE_FINISH, 		/**< Force a write of all received data */
	WIBOCMDCODE_RESET, 			/**< Reset data stream */
	WIBOCMDCODE_EXIT, 			/**< Exit bootloader and jump to application vector */

	/* for example and debugging purposes, define command codes for application to jump into bootloader
	 * and do some actions on LEDs
	 */
	WIBOCMDCODE_XMPLJBOOTL,
	WIBOCMDCODE_XMPLLED
} wibocmdcode_t;

/* this struct is common for all commands and serves as a layer for simple node addressing */
typedef struct {
	uint16_t FCF; 			/**< frame control field */
	uint8_t seqnumber;		/**< sequence number */
	uint16_t destpanid;		/**< destination pan id */
	uint16_t destaddr;		/**< destination short address */
	/* compress source pan id */
	uint16_t srcaddr; 		/**< source short address */
	wibocmdcode_t command; 	/**< the wibo command */
} wibocmd_hdr_t;

typedef struct {
	wibocmd_hdr_t hdr;
} wibocmd_ping_t;

typedef struct {
	wibocmd_hdr_t hdr;
	bootlstatus_t status; 		/**< status code of memory loading process */
	bootlerrorcode_t errcode; 	/**< error code of node */
	uint8_t swversion; 			/**< bootloader software version */
	uint16_t crc; 				/**< checksum of received data stream */
	uint16_t pagesize;			/**< page size */
	const char boardname[];
} wibocmd_pingreply_t;

typedef struct {
	wibocmd_hdr_t hdr;
	uint8_t targmem; 		/**< target memory: EEPROM or FLASH or LOCKBITS */
	uint8_t dsize; 			/**< size of data packet */
	uint8_t data[]; 		/**< data container */
} wibocmd_data_t;

typedef struct {
	wibocmd_hdr_t hdr;
} wibocmd_finish_t;

typedef struct {
	wibocmd_hdr_t hdr;
} wibocmd_reset_t;

typedef struct {
	wibocmd_hdr_t hdr;
} wibocmd_exit_t;

typedef struct {
	wibocmd_hdr_t hdr;
} wibocmd_xmpljbootl_t;

typedef struct {
	wibocmd_hdr_t hdr;
	uint8_t led;
	uint8_t state;
} wibocmd_xmplled_t;

#endif /* WIBOCMD_H_ */
