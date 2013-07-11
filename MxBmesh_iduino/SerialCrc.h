/*
 * SERIALCRC.h
 *
 *  Created on: 2013-5-9
 *      Author: fu
 */

#ifndef SERIALCRC_H_
#define SERIALCRC_H_
#include "stdint.h"
class SERIALCRC {
public:
	static uint16_t crc_byte(uint16_t crc, uint8_t b);
	static uint16_t crc_packet(const uint8_t *data,int len);
	static uint16_t  crc_packet(const uint8_t *data, int offset, int len);

};

#endif /* SERIALCRC_H_ */
