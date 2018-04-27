#include "Arduino.h"
#include <EEPROM.h>
#include <Wire.h>
#include <Modboti2c.h>

void setup()
{

    // Change PWM frequency to 31250
    // https://playground.arduino.cc/Main/TimerPWMCheatsheet

    TCCR1B = TCCR1B & 0b11111000 | 0x01;
    TCCR2B = TCCR2B & 0b11111000 | 0x01;

    //Wire.begin(I2C_ADDRESS);                // join i2c bus with address 0x30
    Serial.begin(9600); // start serial for output
}

void loop()
{
    delay(50);
}