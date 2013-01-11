#ifndef MXS2101_h
#define MXS2101_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"


#define DEFAULT_CAMERA_PACKET_SIZE 20
typedef enum camera_size_t
{
	CAMERA_SIZE_1 = 0x31,
	CAMERA_SIZE_2,
	CAMERA_SIZE_3,
} camera_size_t;
class MXS2101  {
public:

	MXS2101();
	void StartSensor();
	void StopSensor();
	/*
	 * Parse the size of picture.
	 */
	camera_size_t camera_parse_size(char size);
	/*
	 * Take a picture.
	 */
	void camera_take_picture(camera_size_t camera_size, uint16_t packet_size, uint32_t *size, uint16_t *count);
	/*
	 * Get the byte size of the picture.
	 */
	uint32_t camera_get_picture_size();
	/*
	 * Get the total count of the picture's packets.
	 */
	uint16_t camera_get_packet_count();
	/*
	 * Get one packet specified by packet_no.
	 */
	uint16_t camera_get_packet(uint16_t packet_no, uint8_t *buffer);
	/*
	 * Try to get one packet in certain times of tries.
	 */
	uint16_t camera_try_get_packet(uint16_t packet_no, uint8_t *buffer, uint8_t tries);

private:
	void send_frame(uint8_t cmd_char, uint8_t len, uint8_t *payload);
	int refreshdata();
};



#endif
