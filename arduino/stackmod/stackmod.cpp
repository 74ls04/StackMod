#include "Arduino.h"
#include <Wire.h>
#include <Stackmodio.h>

#define I2C_ADDRESS 0x45

/**** Pin Definitions ****/
#define  LF_1 16	//M1
#define  LF_2 17    //M1
#define  LB_1 14    //M2
#define  LB_2 15    //M2
#define  RF_1 7     //M3
#define  RF_2 8     //M3
#define  RB_1 12    //M4
#define  RB_2 13    //M4

#define  M1 3	//M1 PWM
#define  M2 9	//M2 PWM
#define  M3 10	//M3 PWM
#define  M4 11	//M4 PWM

/**** Enums ****/
enum Command_list {
	MOTOR = 77,
	ENABLE = 72
};

enum Motor_cmd {
	FORWARD = 70,
	REVERSE = 71,
	BACK = 66,
	TURN = 84,
	STOP = 83,
	LEFT = 76,
	RIGHT = 82,
	VEER = 86,
	DRIVE = 68,
	CONFIG = 67,
	DISABLE = 73,
	ENABLED = 48,
	DISABLED = 49
};

StackModIO modbot;

// Prototypes
void receive_i2c_packet(int numBytes);
void receive_serial_packet();
void configure_pins();
void set_dir(Motor_cmd motor, Motor_cmd direction);
void drive();




void setup()
{

    // Change PWM frequency to 31250
    // https://playground.arduino.cc/Main/TimerPWMCheatsheet

    TCCR1B = TCCR1B & 0b11111000 | 0x01;
    TCCR2B = TCCR2B & 0b11111000 | 0x01;

    Wire.begin(I2C_ADDRESS);                // join i2c bus with address 0x30
	Wire.onReceive(receive_i2c_packet);
    Serial.begin(115200); // start serial for output
	configure_pins();
}


void drive() {
	int left_speed = modbot.getMotorSpeed(1);
	int right_speed = modbot.getMotorSpeed(2);

	// Set motor directions
	if (left_speed < 0) {
		set_dir(LEFT, REVERSE);
		Serial.println("Reverse");
	}

	if (left_speed == 0) {
		set_dir(LEFT, STOP);
	}

	if (left_speed > 0) {
		Serial.println("Forward");
		set_dir(LEFT, FORWARD);
	}

	if (right_speed < 0) {
		set_dir(RIGHT, REVERSE);
		Serial.println("Reverse");
	}

	if (right_speed == 0) {
		set_dir(RIGHT, STOP);
	}

	if (right_speed > 0) {
		Serial.println("Forward");
		set_dir(RIGHT, FORWARD);
	}


	analogWrite(M1, abs(left_speed));
	analogWrite(M2, abs(left_speed));
	analogWrite(M3, abs(right_speed));
	analogWrite(M4, abs(right_speed));
}

void configure_pins() {
	pinMode(LF_1, OUTPUT);
	pinMode(LF_2, OUTPUT);
	pinMode(LB_1, OUTPUT);
	pinMode(LB_2, OUTPUT);
	pinMode(RF_1, OUTPUT);
	pinMode(RF_2, OUTPUT);
	pinMode(RB_1, OUTPUT);
	pinMode(RB_2, OUTPUT);
	pinMode(M1, OUTPUT);
	pinMode(M2, OUTPUT);
	pinMode(M3, OUTPUT);
	pinMode(M4, OUTPUT);
}


void loop()
{
	//receive_serial_packet();
	drive();
    delay(200);
	Serial.print(modbot.getMotorSpeed(1));
	Serial.print(" ");
	Serial.println(modbot.getMotorSpeed(2));
}

void receive_serial_packet() {
	byte serByte;
	boolean seractive = false;

	while (Serial.available() > 0) {
		seractive = true;
		serByte = modbot.receiveData(Serial.read());
	}

	if (seractive)
	{
		serByte = modbot.receiveData(0); // Reset receive buffer
		seractive = false;
		// Serial.println("Packet received");
	}
}

void receive_i2c_packet(int numBytes) {

	byte i2cByte;
	boolean i2cactive = false;

	while (Wire.available() > 0) {
		i2cactive = true;
		i2cByte = modbot.receiveData(Wire.read());
	}

	if (i2cactive)
	{
		i2cByte = modbot.receiveData(0); // Reset receive buffer
		i2cactive = false;
		// Serial.println("Packet received");
	}
}

void set_dir(Motor_cmd motor, Motor_cmd direction) {

	switch (motor) {

	case LEFT:
		if (direction == FORWARD) {
			digitalWrite(LB_1, 0);
			digitalWrite(LB_2, 1);
			digitalWrite(LF_1, 0);
			digitalWrite(LF_2, 1);
		}
		else if (direction == REVERSE) {
			digitalWrite(LB_1, 1);
			digitalWrite(LB_2, 0);
			digitalWrite(LF_1, 1);
			digitalWrite(LF_2, 0);
		}
		else {
			digitalWrite(LB_1, 0);
			digitalWrite(LB_2, 0);
			digitalWrite(LF_1, 0);
			digitalWrite(LF_2, 0);
		}
	case RIGHT:
		if (direction == FORWARD) {
			digitalWrite(RB_1, 1);
			digitalWrite(RB_2, 0);
			digitalWrite(RF_1, 1);
			digitalWrite(RF_2, 0);
		}
		else if (direction == REVERSE) {
			digitalWrite(RB_1, 0);
			digitalWrite(RB_2, 1);
			digitalWrite(RF_1, 0);
			digitalWrite(RF_2, 1);
		}
		else {
			digitalWrite(LB_1, 0);
			digitalWrite(LB_2, 0);
			digitalWrite(LF_1, 0);
			digitalWrite(LF_2, 0);
		}
	default:
		break;
	}
}
