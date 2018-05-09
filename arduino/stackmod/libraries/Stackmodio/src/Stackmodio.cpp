#include "Arduino.h"
#include <Wire.h>
#include <Stackmodio.h>


// Class Variables //////////////////////////////////////////////////

// Data Registers
unsigned char   StackModIO::motors_reg[MAX_REGISTER_SIZE];
unsigned char   StackModIO::servo_reg[MAX_REGISTER_SIZE];
unsigned char   StackModIO::ultra_reg[MAX_REGISTER_SIZE];
unsigned char   StackModIO::ir_reg[MAX_REGISTER_SIZE];
char            StackModIO::packet_buffer[MAX_BYTES];
boolean         StackModIO::NEW_PACKET = false;
boolean         StackModIO::DEBUGGING = true;
uint8_t         StackModIO::i2c_address = 0;

// Structs and enums //////////////////////////////////////////////////

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
};

static Commands commands[] =
    { //Cmd   //Length //Register address
        "ARM", 1,
        "MTR", 8,
        "SRV", 8,
        "ULT", 8,
        "IRS", 8,
        "DGT", 1,
        "ANL", 1,
        0
    };


struct MOTORS {
	int motor_1;
	int motor_2;
	int motor_3;
	int motor_4;
	int motor_5;
	int motor_6;
	int motor_7;
	int motor_8;
};

// typdefs that allor for convenient access to our data registers
typedef struct
{
    int motor_1;
    int motor_2;
    int motor_3;
    int motor_4;
    int motor_5;
    int motor_6;
    int motor_7;
    int motor_8;
} motors_t;

typedef struct
{
    unsigned short val_1 : 16;
    unsigned short val_2 : 16;
    unsigned short val_3 : 16;
    unsigned short val_4 : 16;
    unsigned short val_5 : 16;
    unsigned short val_6 : 16;
    unsigned short val_7 : 16;
    unsigned short val_8 : 16;
} large_pck_t;



void receiveEvent(int howMany)
{
    static boolean receiving = false;
    static byte index = 0;
    char startMarker = '{';
    char endMarker = '}';
    char inByte;

    while (Wire.available() > 0 &&  StackModIO::NEW_PACKET == false)
    {

        inByte = Wire.read();

        if (receiving == true && inByte != startMarker)
        {
            if (inByte != endMarker)
            {
                StackModIO::packet_buffer[index] = inByte;
                index++;
                if (index >= MAX_BYTES)
                {
                    index = MAX_BYTES - 1;
                }
            }
            else
            {
                StackModIO::packet_buffer[index] = '\0'; // terminate the string
                receiving = false;
                index = 0;
                StackModIO::NEW_PACKET = true;
            }
        }
        else if (inByte == startMarker)
        {
            receiving = true;
            memset(StackModIO::packet_buffer, 0, MAX_BYTES); // Empty char buffer
        }
    }

}



// Public Methods ///////////////////////////////////////////////////////////////

void StackModIO::begin(uint8_t slave_address)
{
    i2c_address = slave_address;
    Wire.begin(slave_address); 
	Wire.onReceive(receiveEvent); 
	// Wire.onRequest(requestEvent);
}

// Private Methods //////////////////////////////////////////////////////////////

/*
void get_speeds()
{
    motors_t *motor_speeds = (motors_t *)motors_reg;

    Serial.println((int)motor_speeds->motor_1_speed);
    Serial.println((int)motor_speeds->motor_5_speed);
}

 int StackModIO::getMotorSpeed(int motor_number) 
 {
    small_pckt_t *motor_speeds = (small_pckt_t *) motors_reg;
    Serial.println((int)motor_speeds->val_1);
 }   

 */

void StackModIO::processPacket()
{
    //serial.println("HB");
    char data[MAX_REGISTER_SIZE + 1];

    if (NEW_PACKET)
    {
        // Clear the packet status
        NEW_PACKET = false;

        serial.println(packet_buffer);

        // Validate minimal packet size
        unsigned char packet_size = strlen(packet_buffer);

        if (packet_size < 5)
        {
            serial.println("Invalid packet size ");
            return;
        }

        // Get target address of packet
        uint8_t address = packet_buffer[0];


        // Check if command or query
        uint8_t action = packet_buffer[1];

        // Identify which command was sent
        int cmd;
        for (cmd = 0; commands[cmd].cmd; cmd++)
        {
            if (!strncmp(&packet_buffer[2], commands[cmd].cmd, strlen(commands[cmd].cmd)))
                break;
        }

        // Address must match our address
        
        if (address != i2c_address) { 
            if (DEBUGGING) Serial.println("INVALID ADDRESS");
            return;
        }
        

        switch (cmd)
        {
        case MOTOR:

            if (DEBUGGING)
                serial.println("This is a motor command");

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
                    serial.println("Invalid motor string length");
            }

            //Serial.println(motors_reg);
            break;
        case ULTRASONIC:
            if (DEBUGGING)
                serial.println("This is an ultrasonic command");
            break;
        default:
            serial.println("Command not found");
            break;
        }

        // if (DEBUGGING) Serial.print(motorSpeeds[0]);
        // if (DEBUGGING) Serial.print(motorSpeeds[1]);
        // if (DEBUGGING) Serial.print(motorSpeeds[2]);
        // if (DEBUGGING) Serial.print(motorSpeeds[3]);
        /*
        for (int i = 0; i < packet_size; i++) {
            
        } */
    }
}