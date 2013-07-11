/*
 * SERIALCRC.cpp
 *
 *  Created on: 2013-5-9
 *      Author: fu
 */

#include "SerialCrc.h"



 uint16_t   SERIALCRC::crc_byte(uint16_t crc, uint8_t b)
{
	uint8_t i;

	crc = (uint16_t)(crc ^ b << 8);
	i = 8;
	do
	{
		if ((crc & 0x8000) > 0)
			crc = (uint16_t)(crc << 1 ^ 0x1021);
		else
			crc = (uint16_t)(crc << 1);
	}
	while (--i != 0);

	return crc;
}
 uint16_t   SERIALCRC::crc_packet(const uint8_t *data,int len)
{
	uint16_t crc = 0;
		int dataIndex = len;
	while (dataIndex-- > 0)
		crc = crc_byte(crc, data[len - dataIndex - 1]);

	return crc;
}
 uint16_t    SERIALCRC::crc_packet(const uint8_t *data, int offset, int len)
{
	uint16_t crc = 0;
	int dataIndex = len;
	while (dataIndex-- > 0)
		crc = crc_byte(crc, data[offset + len - dataIndex - 1]);

	return crc;
}
