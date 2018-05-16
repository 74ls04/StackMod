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

    void            processPacket();




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