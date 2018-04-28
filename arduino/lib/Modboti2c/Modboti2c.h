#ifndef Modboti2c_h
#define Modboti2c_h

#include "Arduino.h"

#define MAX_BYTES 32
#define MAX_REGISTER_SIZE 32


class Modboti2c
{
  private:
    static unsigned char motors_reg[];
    static unsigned char servo_reg[];
    static unsigned char ultra_reg[];
    static unsigned char ir_reg[];

    static char packet_buffer[];

    static uint8_t i2c_address;
    static boolean NEW_PACKET;
    static boolean DEBUGGING;

    void processPacket();
    void receiveEvent(int howMany);
    void requestEvent();
    bool initTheWireCommunication(int ownMasterAddress);

    Stream& serial;

    public:
        Modboti2c(Stream& s = Serial) : serial(s){}
        void begin(uint8_t);
};

#endif