#ifndef _MODBOTI2C_h
#define _MODBOTI2C_h
#include "Arduino.h"

class Modbot
{
    public:
        Modbot(int slave_address);

  private:

    struct Commands;
    static Commands commands[];
    
    void receiveEvent(int howMany);
    void requestEvent();
    bool initTheWireCommunication(int ownMasterAddress);
    void begin();
};

#endif