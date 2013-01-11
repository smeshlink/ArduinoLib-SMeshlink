/*
 * wibocmd.h
 *
 *  Created on: 12.01.2010
 *      Author: dthiele
 */

#ifndef WIBO_NODEADDR_H_
#define WIBO_NODEADDR_H_

#include <stdint.h>

/*
 * data fields to be located at the end of flash (FLASHEND - sizeof(wibo_nodeaddr_t))
 *
 * 16 bytes to be allocated, reserve the very firt bytes for future use
 */
typedef struct{
	uint8_t _reserved_0;
	uint8_t _reserved_1;
	uint8_t _reserved_2;
	uint8_t _reserved_3;
	uint16_t panid;
	uint16_t short_addr;
	uint64_t mac_addr;
}wibo_nodecfg_t;

#endif /* WIBO_NODEADDR_H_ */
