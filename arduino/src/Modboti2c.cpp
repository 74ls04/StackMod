#include "Arduino.h"
#include <Wire.h>
#include <Modboti2c.h>

#define MAX_BYTES 32
#define I2C_ADDRESS 0x38

#define MAX_REGISTER_SIZE 32

// Data Registers
char motors_reg[MAX_REGISTER_SIZE];
char servo_reg[MAX_REGISTER_SIZE];
char ultra_reg[MAX_REGISTER_SIZE];
char ir_reg[MAX_REGISTER_SIZE];

boolean NEW_PACKET = false;
boolean DEBUGGING = true;

char packet_buffer[MAX_BYTES];

void setup()
{

    // Change PWM frequency to 31250
    // https://playground.arduino.cc/Main/TimerPWMCheatsheet

    TCCR1B = TCCR1B & 0b11111000 | 0x01;
    TCCR2B = TCCR2B & 0b11111000 | 0x01;

    //Wire.begin(I2C_ADDRESS);                // join i2c bus with address 0x30
    Serial.begin(9600); // start serial for output
}

class Modbot
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

    struct Commands
    {
        char *cmd;              // cmd
        unsigned char data_len; // length
        unsigned char *addr;
    };

    // typdefs that allor for convenient access to our data registers
    typedef struct
    {
        char motor_1_speed;
        char motor_2_speed;
        char motor_3_speed;
        char motor_4_speed;
        char motor_5_speed;
        char motor_6_speed;
        char motor_7_speed;
        char motor_8_speed;
    } Motors;

    typedef struct
    {
        unsigned char servo_1_pos;
        unsigned char servo_2_pos;
        unsigned char servo_3_pos;
        unsigned char servo_4_pos;
        unsigned char servo_5_pos;
        unsigned char servo_6_pos;
        unsigned char servo_7_pos;
        unsigned char servo_8_pos;
    } Servos;

    typedef struct
    {
        unsigned char sonar_1_dist : 16;
        unsigned char sonar_2_dist : 16;
        unsigned char sonar_3_dist : 16;
        unsigned char sonar_4_dist : 16;
        unsigned char sonar_5_dist : 16;
        unsigned char sonar_6_dist : 16;
        unsigned char sonar_7_dist : 16;
        unsigned char sonar_8_dist : 16;
    } Ultrasonics;
    
    typedef struct
    {
        unsigned char ir_1_dist : 16;
        unsigned char ir_2_dist : 16;
        unsigned char ir_3_dist : 16;
        unsigned char ir_4_dist : 16;
        unsigned char ir_5_dist : 16;
        unsigned char ir_6_dist : 16;
        unsigned char ir_7_dist : 16;
        unsigned char ir_8_dist : 16;
    } Irs;

    static Commands commands[];
};

Modbot::Commands Modbot::commands[] =
    { //Cmd   //Length //Register address
        "ARM", 1, "0x01",
        "MTR", 8, "0x02",
        "SRV", 8, "0x03",
        "ULT", 8, "0x04",
        "IRS", 8, "0x05",
        "DGT", 1, "0x06",
        "ANL", 1, "0x07",
        0
    };

void receive_serial_packet()
{
    static boolean receiving = false;
    static byte index = 0;
    char startMarker = '{';
    char endMarker = '}';
    char inByte;

    while (Serial.available() > 0 && NEW_PACKET == false)
    {

        inByte = Serial.read();

        if (receiving == true && inByte != startMarker)
        {
            if (inByte != endMarker)
            {
                packet_buffer[index] = inByte;
                index++;
                if (index >= MAX_BYTES)
                {
                    index = MAX_BYTES - 1;
                }
            }
            else
            {
                packet_buffer[index] = '\0'; // terminate the string
                receiving = false;
                index = 0;
                NEW_PACKET = true;
            }
        }
        else if (inByte == startMarker)
        {
            receiving = true;
            memset(packet_buffer, 0, MAX_BYTES); // Empty char buffer
        }
    }
}

void get_speeds()
{
    Motors *motor_speeds = (Motors *)motors_reg;

    Serial.println((int)motor_speeds->motor_1_speed);
    Serial.println((int)motor_speeds->motor_5_speed);
}

void process_packet()
{

    char data[MAX_REGISTER_SIZE + 1];

    if (NEW_PACKET == true)
    {

        // Clear the packet status
        NEW_PACKET = false;

        if (DEBUGGING)
            Serial.println(packet_buffer);

        // Validate minimal packet size
        byte packet_size = strlen(packet_buffer);

        if (packet_size < 5)
        {
            Serial.print("Invalid packet size ");
            return;
        }

        // Get target address of packet
        // byte address = packet_buffer[0];

        // Check if command or query
        // char action = packet_buffer[1];

        // Identify which command was sent
        int cmd;
        for (cmd = 0; commands[cmd].cmd; cmd++)
        {
            if (!strncmp(&packet_buffer[2], commands[cmd].cmd, strlen(commands[cmd].cmd)))
                break;
        }

        // Address must match our address
        /*
        if (address != I2C_ADDRESS) { 
            if (DEBUGGING) Serial.println("INVALID ADDRESS");
            return;
        }
        */

        switch (cmd)
        {
        case MOTOR:

            if (DEBUGGING)
                Serial.println("This is a motor command");

            // Copy packet to temp array and verify length
            snprintf(data, commands[cmd].data_len + 1, "%s", &packet_buffer[5]);

            if (strlen(data) == commands[cmd].data_len)
            {
                memcpy(motors_reg, data, sizeof(data));

                // Clean up all the temp data
                memset(data, 0, sizeof(data));
                memset(packet_buffer, 0, MAX_BYTES); // Empty char buffer
            }
            else
            {
                if (DEBUGGING)
                    Serial.println("Invalid motor string length");
            }

            //Serial.println(motors_reg);
            break;
        case ULTRASONIC:
            if (DEBUGGING)
                Serial.println("This is an ultrasonic command");
            break;
        default:
            Serial.println("Command not found");
            break;
        }

        get_speeds();
        // if (DEBUGGING) Serial.print(motorSpeeds[0]);
        // if (DEBUGGING) Serial.print(motorSpeeds[1]);
        // if (DEBUGGING) Serial.print(motorSpeeds[2]);
        // if (DEBUGGING) Serial.print(motorSpeeds[3]);
        /*
        for (int i = 0; i < packet_size; i++) {
            
        } */
    }
}

void loop()
{
    receive_serial_packet();
    process_packet();
    delay(50);
}