#include "Arduino.h"
#include <Wire.h>
#include <Stackmodio.h>

#define I2C_ADDRESS 0x45

StackModIO modbot;

// Prototypes
void receive_i2c_packet(int numBytes);
void receive_serial_packet();

void setup()
{

    // Change PWM frequency to 31250
    // https://playground.arduino.cc/Main/TimerPWMCheatsheet

    TCCR1B = TCCR1B & 0b11111000 | 0x01;
    TCCR2B = TCCR2B & 0b11111000 | 0x01;

    Wire.begin(I2C_ADDRESS);                // join i2c bus with address 0x30
	Wire.onReceive(receive_i2c_packet);
    Serial.begin(9600); // start serial for output
}



void loop()
{
	receive_serial_packet();
    delay(1000);
	Serial.println(modbot.current_command);
}

void receive_serial_packet() {
	byte serByte;

	while (Serial.available() > 0) {
		serByte = modbot.receiveData(Serial.read());
	}
}

void receive_i2c_packet(int numBytes) {

	byte inByte;

	// if (Serial.available() > 0) {

	while (Wire.available() > 0) {
		inByte = modbot.receiveData(Wire.read());
	}
}