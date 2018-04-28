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

    static uint8_t i2c_address;

    

    Stream& serial;

    public:
        Modboti2c(Stream& s = Serial) : serial(s){}
        void begin(uint8_t);
        // static void receiveEvent(int howMany);
        // static void requestEvent();
      void processPacket();
      int getMotorSpeed(int motor_number);
      static char packet_buffer[];
      static boolean NEW_PACKET;
      static boolean DEBUGGING;
};

#endif