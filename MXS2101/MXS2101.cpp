#include "MXS2101.h"
#include "Wire.h"
#define CONTROLCHIPADDRESS  0x27
#define CAMERA_CMD_H 0x48
#define CAMERA_CMD_R 0x52
#define CAMERA_CMD_E 0x45
#define CAMERA_CMD_F 0x46

#define SOF_CHAR 0x55
#define EOF_CHAR 0x23

#define MAX_DELAY 100000
#define CMD_LEN 128
static struct {
	uint8_t cmd;
	union {
		uint32_t size;
		struct {
			uint16_t no;
			uint16_t len;
		};
	};
	uint8_t frame[CMD_LEN];
	uint16_t checksum;
	uint16_t count;
	uint16_t ndx;
	uint8_t done;
} cmd;

void MXS2101::send_frame(uint8_t cmd_char, uint8_t len, uint8_t *payload)
{
	uint8_t i = 0;
	cmd.done = 0;
	Serial.write(SOF_CHAR);
	Serial.write(cmd_char);
	for (; i < len; i++) {
		Serial.write(*payload++);
	}
	cmd.ndx = 0;
	Serial.write(EOF_CHAR);
	Serial.flush();
	//delay(400);
}

MXS2101::MXS2101()
{




}
void MXS2101::StartSensor()
{
	Wire.begin(); // set up SEABO I2C support
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x03);
	Wire.write((byte) 0x00);
	Wire.endTransmission(); // leave I2C bus
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x81); //enable 5V and uart0
	Wire.endTransmission(); // leave I2C bus
#if	 defined(isant2400cc) || defined(mx231cc)
	pinMode(28, INPUT); //A4
	pinMode(13, INPUT); //D5
	pinMode(0, INPUT); //B0
#elif	 defined(isant900cb)
	pinMode(31, INPUT); //PB7
	pinMode(35, INPUT); //E3
#endif
	Serial.begin(38400);
}

void MXS2101::StopSensor()
{
	Wire.begin(); // set up SEABO I2C support
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x03);
	Wire.write((byte) 0x00);
	Wire.endTransmission(); // leave I2C bus
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x00); //enable 5V uart1
	Wire.endTransmission(); // leave I2C bus

}
camera_size_t MXS2101::camera_parse_size(char size)
{
	if (CAMERA_SIZE_2 == size) {
		return CAMERA_SIZE_2;
	} else if (CAMERA_SIZE_3 == size) {
		return CAMERA_SIZE_3;
	} else {
		return CAMERA_SIZE_1;
	}
}
int MXS2101::refreshdata()
{
	while (Serial.available())
	{

		char ch=Serial.read();
		switch (cmd.ndx) {
		case 0:
			/* 1st byte, must be 0x55/'U'. */
			cmd.done = 0;
			if (SOF_CHAR != ch) {
				return 0;
			}
			break;
		case 1:
			/* 2nd byte, command byte. */
			cmd.cmd = ch;
			break;
		case 2:
			/* 3rd byte, 0x23/'#' (response of H/E) or 1st byte of size. */
			if ((CAMERA_CMD_H == cmd.cmd || CAMERA_CMD_E == cmd.cmd) && EOF_CHAR == ch) {
				/* End of command */
				cmd.done = 1;
				cmd.ndx = 0;
				return 0;
			} else {
				/* 1st byte of number */
				cmd.size = (uint32_t)ch & 0x000000FF;
			}
			break;
		case 3:
			/* 4th byte, 2nd byte of size. */
			cmd.size += ((uint32_t)ch & 0x000000FF) << 8;
			break;
		case 4:
			/* 5th byte, 3rd byte of size. */
			cmd.size += ((uint32_t)ch & 0x000000FF) << 16;
			break;
		case 5:
			/* 6th byte, 4th byte of size. */
			cmd.size += ((uint32_t)ch & 0x000000FF) << 24;
			break;
		case 6:
		case 7:
			cmd.frame[cmd.ndx - 6] = ch;
			break;
		case 8:
			/* 9th byte, might be '#' (response of R). */
			if (CAMERA_CMD_R == cmd.cmd && EOF_CHAR == ch) {
				cmd.done = 1;
				cmd.ndx = 0;
				return 0;
			} else {
				cmd.frame[cmd.ndx - 6] = ch;
			}
			break;
		default:
			/* Payload and checksum. */
			if (cmd.ndx >= cmd.len + 6) {
				/* All done, check checksum. */
				cmd.done++;
				cmd.frame[cmd.ndx - 6] = ch;
				if (cmd.ndx >= cmd.len + 7) {
					/* 2nd byte of checksum */
					cmd.checksum += ((uint16_t)ch & 0x00FF) << 8;
					/* Check the checksum */
					uint16_t sum = 0, ii = 0;
					for (; ii < cmd.len; ii++) {
						sum += cmd.frame[ii];
					}
					if (sum == cmd.checksum) {
						cmd.done = 1;
					}
					cmd.ndx = 0;
					return 0;
				} else {
					/* 1st byte of checksum. */
					cmd.checksum = ch;
				}
			} else {
				/* Store data. */
				cmd.frame[cmd.ndx - 6] = ch;
			}
			break;
		}

		cmd.ndx++;
		return 0;

	}
	return 0;
}
void MXS2101::camera_take_picture(camera_size_t camera_size, uint16_t packet_size, uint32_t *size, uint16_t *count)
{
	uint32_t counter = MAX_DELAY;
	uint8_t payload[3];
	payload[0] = camera_size;
	payload[1] = packet_size & 0xFF;
	payload[2] = (packet_size >> 8) & 0xFF;
	cmd.count = 0;
	do
	{
		send_frame(CAMERA_CMD_H, 3, payload);
		while ((!cmd.done || CAMERA_CMD_R != cmd.cmd) && counter--) {
			/* Waiting for response. */
			/* A trick for enabling current loop to be interrupted. */
			if (counter==0) break;
			*count = 0;
			refreshdata();
		}
	} while (!cmd.done);
	*size = camera_get_picture_size();
	*count = camera_get_packet_count();

}

uint32_t MXS2101::camera_get_picture_size()
{

	if (CAMERA_CMD_R == cmd.cmd && cmd.done) {
		return cmd.size;
	} else {
		return 0;
	}
}

uint16_t MXS2101::camera_get_packet_count()
{
	if (CAMERA_CMD_R == cmd.cmd && cmd.done && !cmd.count) {
		cmd.count = cmd.frame[0] + ((cmd.frame[1] & 0xFF) << 8);
	}
	return cmd.count;
}
uint16_t MXS2101::camera_get_packet(uint16_t packet_no, uint8_t *buffer)
{
	uint8_t payload[2];
	uint16_t i = 0, count = camera_get_packet_count();
	uint32_t counter = MAX_DELAY;

	if (0 == packet_no || packet_no > count) {
		return 0;
	}

	payload[0] = packet_no;
	payload[1] = packet_no >> 8;

	send_frame(CAMERA_CMD_E, 2, payload);
	while ((!cmd.done || CAMERA_CMD_F != cmd.cmd) && counter--) {
		/* Waiting for response. */
		/* A trick for enabling current loop to be interrupted. */
		*buffer = 0;
		refreshdata();
	}
	if (!cmd.done) {
		return 0;
	}
	for (; i < cmd.len; i++) {
		buffer[i] = cmd.frame[i];
	}
	//#if LOWPOWER
	// setcansleepstate(1);
	//#endif
	return cmd.len;
}
uint16_t MXS2101::camera_try_get_packet(uint16_t packet_no, uint8_t *buffer, uint8_t tries)
{
	uint16_t len = 0;
	while (tries--) {
		len = camera_get_packet(packet_no, buffer);
		if (len > 0)
			return len;
	}
	return 0;
}
