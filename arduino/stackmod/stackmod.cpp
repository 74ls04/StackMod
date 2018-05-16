#include "Arduino.h"
#include <Wire.h>
#include <Stackmodio.h>

#define I2C_SLAVE_ADDRESS 0x45

/**** Pin Definitions ****/
#define  LF_1 16    //M1
#define  LF_2 17    //M1
#define  LB_1 14    //M2
#define  LB_2 15    //M2
#define  RF_1 7     //M3
#define  RF_2 8     //M3
#define  RB_1 12    //M4
#define  RB_2 13    //M4
#define  M1 3       //M1 PWM
#define  M2 9       //M2 PWM
#define  M3 10      //M3 PWM
#define  M4 11      //M4 PWM

/**** Enums ****/

enum Motor_cmd {
    FORWARD,
    REVERSE,
    BACK,
    TURN,
    STOP,
    LEFT,
    RIGHT,
    VEER,
    DRIVE,
    CONFIG,
    DISABLE,
    ENABLED,
    DISABLED,
    MOTOR,
    ENABLE
};

StackModIO modbot;

// Prototypes
void receivei2cPacket(int numBytes);
void receiveSerialPacket();
void configurePins();
void setDir(Motor_cmd motor, Motor_cmd direction);
void drive();
void forwardDir();
void backDir();
void turnLeftDir();
void turnRightDir();
void setBrake();


void setup()
{

    // Change PWM frequency to 31250
    // https://playground.arduino.cc/Main/TimerPWMCheatsheet

    TCCR1B = TCCR1B & 0b11111000 | 0x01;
    TCCR2B = TCCR2B & 0b11111000 | 0x01;

    Wire.begin(I2C_SLAVE_ADDRESS);                // join i2c bus with address 0x30
    Wire.onReceive(receivei2cPacket);

    // Disable internal I2C pullups
    digitalWrite(SDA, 0);
    digitalWrite(SCL, 0);

    Serial.begin(115200); // start serial for output
    configurePins();
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


void drive() {
    int left_speed = modbot.getMotorSpeed(1);
    int right_speed = modbot.getMotorSpeed(2);

    // Set motor directions
    if (left_speed < 0) 
    {
        setDir(LEFT, REVERSE);
    } else if (left_speed == 0) 
    {
        setDir(LEFT, STOP);
    } else 
    {
        setDir(LEFT, FORWARD);
    }

    if (right_speed < 0) {
        setDir(RIGHT, REVERSE);
    } else if (right_speed == 0) 
    {
        setDir(RIGHT, STOP);
    } else 
    {
        setDir(RIGHT, FORWARD);
    }

    // Write PWM values
    analogWrite(M1, abs(left_speed));
    analogWrite(M2, abs(left_speed));
    analogWrite(M3, abs(right_speed));
    analogWrite(M4, abs(right_speed));
}


void configurePins() {
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


void receiveSerialPacket() {
    uint8_t ser_byte;
    boolean ser_active = false;

    while (Serial.available() > 0) {
        ser_active = true;
        ser_byte = modbot.receiveData(Serial.read());
    }

    if (ser_active)
    {
        ser_byte = modbot.receiveData(0); // Reset receive buffer
        ser_active = false;
        // Serial.println("Packet received");
    }
}

void receivei2cPacket(int numBytes) {

    uint8_t i2c_byte;
    boolean i2c_active = false;

    while (Wire.available() > 0) {
        i2c_active = true;
        i2c_byte = modbot.receiveData(Wire.read());
    }

    if (i2c_active)
    {
        i2c_byte = modbot.receiveData(0); // Reset receive buffer
        i2c_active = false;
        // Serial.println("Packet received");
    }
}

// Manual control function

void setDir(Motor_cmd motor, Motor_cmd direction) {

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
        break;
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
            digitalWrite(RB_1, 0);
            digitalWrite(RB_2, 0);
            digitalWrite(RF_1, 0);
            digitalWrite(RF_2, 0);
        }
        break;
    default:
        break;
    }
}


// Autonomous control functions
void forwardDir() {
    digitalWrite(LB_1, 0);
    digitalWrite(LB_2, 1);
    digitalWrite(LF_1, 0);
    digitalWrite(LF_2, 1);

    digitalWrite(RB_1, 1);
    digitalWrite(RB_2, 0);
    digitalWrite(RF_1, 1);
    digitalWrite(RF_2, 0);
}

void backDir() {
    digitalWrite(LB_1, 1);
    digitalWrite(LB_2, 0);
    digitalWrite(LF_1, 1);
    digitalWrite(LF_2, 0);

    digitalWrite(RB_1, 0);
    digitalWrite(RB_2, 1);
    digitalWrite(RF_1, 0);
    digitalWrite(RF_2, 1);
}

void turnLeftDir() {
    digitalWrite(LB_1, 1);
    digitalWrite(LB_2, 0);
    digitalWrite(LF_1, 1);
    digitalWrite(LF_2, 0);

    digitalWrite(RB_1, 1);
    digitalWrite(RB_2, 0);
    digitalWrite(RF_1, 1);
    digitalWrite(RF_2, 0);
}

void turnRightDir() {
    digitalWrite(LB_1, 0);
    digitalWrite(LB_2, 1);
    digitalWrite(LF_1, 0);
    digitalWrite(LF_2, 1);

    digitalWrite(RB_1, 0);
    digitalWrite(RB_2, 1);
    digitalWrite(RF_1, 0);
    digitalWrite(RF_2, 1);
}

void setBrake() {
    digitalWrite(LB_1, 1);
    digitalWrite(LB_2, 1);
    digitalWrite(LF_1, 1);
    digitalWrite(LF_2, 1);

    digitalWrite(RB_1, 1);
    digitalWrite(RB_2, 1);
    digitalWrite(RF_1, 1);
    digitalWrite(RF_2, 1);
}