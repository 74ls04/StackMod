/*
# StackMod: The Modular Robot Protocol
Author: Paul Bupe Jr


Intro description...
## Serial Message Format

**\<START>\<SRC_ADDRESS>\<DEST_ADDRESS>\<COMMAND>\<DATA>\<END>\<CHECKSUM>**

* The START byte is ASCII "{" (123 decimal, 0x7B).
* The SRC_ADDRESS byte is any value from 64 to 95 decimal(0x40 to 0x5F, ASCII “@” to “_”).
* The DEST_ADDRESS byte is any value from 64 to 95 decimal(0x40 to 0x5F, ASCII “@” to “_”).
* The ACTION byte is an ASCII “ ? ” for GET and ASCII “$” for SET.
* The COMMAND bytes are three characters.
* The DATA bytes are variable length and are described in the Data section.
* The END byte is ASCII "}" (125 decimal, 0x7D).
* The CHECKSUM is calculated by subtracting 32 from all the characters in the packet(excluding the checksum) and summing them.The modulo 95 of this value is then calculated and 32 is added back to that value.


### Example Query and Response
> Query:    **{@p?MTR2}X**
>Response:  **{p@?MTR2+078}C**

## Available Commands
| Command | Description|
|------ - | ------------ - |
|ARM | Arm / Disarm Motors  |
|MTR | Motor       |
|SRV | Servo	      |
|ULT | Ultrasonic  |
|IRS | IR          |
|DGT | Digital Pin |
|ANL | Analog Pin |
*/

#ifndef Stackmodio_h
#define Stackmodio_h

#include "Arduino.h"

#define MAX_BYTES 32
#define MAX_REGISTER_SIZE 32
#define MAX_MODULES 15

class StackModIO
{
private:
    // Available commands
    enum Cmd_ids
    {
        ARM_MOTORS,
        MOTOR,
        SERVO,
        ULTRASONIC,
        IR,
        DIGITAL,
        ANALOG,
        NUM_COMMANDS
    };

    enum Actions
    {
        COMMAND = 36,
        QUERY = 63
    };

    struct Commands
    {
        char *cmd;              // cmd
        unsigned char cmd_len; // command length
        unsigned char qry_len; // query length

    };

    static int      motors_reg[];
    static uint8_t  servo_reg[];
    static uint8_t  ultra_reg[];
    static uint8_t  ir_reg[];
    static int      motors[];
    static char     packet_buffer[];
    static Commands commands[];
    static uint8_t  i2c_address;

    static bool     NEWPACKET;
    static bool     DEBUGGING;
    static bool     RECEIVING;

    Stream&         serial;

    void            processPacket(char *);
    int             calculateChecksum(String packet);



public:
    StackModIO(Stream& s = Serial) : serial(s) {}
    void begin(uint8_t);
    // static void receiveEvent(int howMany);
    // static void requestEvent();

    void            setMotorSpeed(uint8_t motor_number, int speed);
    void            setMotorRange(int min, int max);
    int             getMotorSpeed(uint8_t motor_number);

    uint8_t         receiveData(uint8_t inByte);
    static char     current_command[];

};

#endif