#include "MXN820.h"
#include "Wire.h"
#define CONTROLCHIPADDRESS  0x27
 unsigned char arduino_node_id[8];

MXN820::MXN820() {

}
void MXN820::BuzzerID() {
	int count = 0;
	Wire.begin(); // set up SEABO I2C support
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x03);
	Wire.write((byte) 0x00);
	Wire.endTransmission(); // leave I2C bus
	count = arduino_node_id[7] / 0x10;
	for (int i = 0; i < count; i++) {
		buzzer_long();
	}
	buzzer_short();
	count = arduino_node_id[7] % 0x10;
	for (int i = 0; i < count; i++) {
		buzzer_long();
	}
}
int MXN820::GetBatteryVoltage() {
	int i = 0;
	return i;

}
int MXN820::GetChargeVoltage() {
	int i = 0;
	return i;
}

void MXN820::buzzer_long() {
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x40); //0x80 enable5V  0x8 disable UART1
	Wire.endTransmission(); // leave I2C bus
	delay(300);
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x00); //0x80 enable5V  0x8 disable UART1
	Wire.endTransmission(); // leave I2C bus
	delay(300);
}
void MXN820::buzzer_short() {
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x40); //0x80 enable5V  0x8 disable UART1
	Wire.endTransmission(); // leave I2C bus
	delay(40);
	Wire.beginTransmission(CONTROLCHIPADDRESS); // join I2C
	Wire.write(0x01);
	Wire.write((byte) 0x00); //0x80 enable5V  0x8 disable UART1
	Wire.endTransmission(); // leave I2C bus
	delay(1000);
}
