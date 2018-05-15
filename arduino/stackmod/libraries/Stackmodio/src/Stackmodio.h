#ifndef Stackmodio_h
#define Stackmodio_h

#include "Arduino.h"

#define MAX_BYTES 32
#define MAX_REGISTER_SIZE 32

class StackModIO
{
private:
	static byte		motors_reg[];
	static byte		servo_reg[];
	static byte		ultra_reg[];
	static byte		ir_reg[];
	static uint8_t	motors[];

	static char		packet_buffer[];

	static uint8_t	i2c_address;

	static bool		NEWPACKET;
	static bool		DEBUGGING;
	static bool		RECEIVING;

	Stream&			serial;

	void			processPacket();

public:
	StackModIO(Stream& s = Serial) : serial(s) {}
	void begin(uint8_t);
	// static void receiveEvent(int howMany);
	// static void requestEvent();

	void			setMotorSpeed(uint8_t motor_number, byte speed);
	void			setMotorRange(int min, int max);
	int				getMotorSpeed(uint8_t motor_number);

	byte			receiveData(byte inByte);
	static char		current_command[];

};

#endif