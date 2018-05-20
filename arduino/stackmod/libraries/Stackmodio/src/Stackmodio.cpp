/*
# StackMod: The Modular Robot Protocol
Author : Paul Bupe Jr


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
>Response:  **{p@?MTR2 + 078}C**

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

#include "Stackmodio.h"
#include "Arduino.h"
#include <Stackmodio.h>


// Class Variables //////////////////////////////////////////////////

// Data Registers
int             StackModIO::motors_reg[MAX_MODULES] = { 0 };
uint8_t         StackModIO::servo_reg[MAX_REGISTER_SIZE];
uint8_t         StackModIO::ultra_reg[MAX_REGISTER_SIZE];
uint8_t         StackModIO::ir_reg[MAX_REGISTER_SIZE];
char            StackModIO::packet_buffer[MAX_BYTES] = { 0 };
char            StackModIO::current_command[MAX_BYTES] = { 0 };
int             StackModIO::motors[MAX_MODULES] = { 0 };

bool            StackModIO::NEWPACKET = false;
bool            StackModIO::DEBUGGING = true;
bool            StackModIO::RECEIVING = false;
uint8_t         StackModIO::i2c_address = 0;

// Structs and enums //////////////////////////////////////////////////


StackModIO::Commands StackModIO::commands[] =
    { //Cmd   //Length //Register address
        "ARM", 1 , 7,
        "MTR", 11, 7,
        "SRV", 11, 7,
        "ULT", 11, 7,
        "IRS", 11, 7,
        "DGT", 1 , 7,
        "ANL", 1 , 7,
        0
    };


/*
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

*/

uint8_t StackModIO::receiveData(uint8_t inByte)
{
    static uint8_t index = 0;
    static bool CHECKSUM = false;
    char startMarker = '{';
    char endMarker = '}';

    if (inByte == 0)
    {
        memset(packet_buffer, 0, MAX_BYTES); // Empty char buffer
        RECEIVING = false;
        index = 0;
    }

    
    if (NEWPACKET && CHECKSUM)
    {
        int calculated_checksum = calculateChecksum(packet_buffer);
        //serial.print(inByte);
        //serial.print(" ");
        //serial.println(calculated_checksum);
        if (calculated_checksum == inByte) {
            memcpy(current_command, packet_buffer, sizeof(packet_buffer));
            memset(packet_buffer, 0, MAX_BYTES); // Empty char buffer
            processPacket(current_command);
        }
        else {
            memset(packet_buffer, 0, MAX_BYTES); // Empty char buffer
        }
        CHECKSUM = false;
        NEWPACKET = false;
        return inByte;
    }
    
    //{Ae$MTR1+139}Y

    if (!NEWPACKET)
    {

        if (RECEIVING && inByte != startMarker)
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
                index = 0;
                RECEIVING = false;
                NEWPACKET = true;
                CHECKSUM = true;
                //serial.println("End of packet");
            }
        }
        else if (inByte == startMarker)
        {
            RECEIVING = true;
            memset(packet_buffer, 0, MAX_BYTES); // Empty char buffer
            //serial.println("Start of packet");
        }
    }


    return inByte;
}



// Public Methods ///////////////////////////////////////////////////////////////

void StackModIO::begin(uint8_t slave_address)
{
    i2c_address = slave_address;
    // Wire.begin(slave_address); 
    // Wire.onReceive(receiveEvent); 
    // Wire.onRequest(requestEvent);
}

void StackModIO::setMotorSpeed(uint8_t motor_number, int speed)
{
    if (motor_number > 0 && motor_number < MAX_MODULES + 1)
    {
        motors_reg[motor_number - 1] = speed;
    }
}

void StackModIO::setMotorRange(int min, int max)
{
}

int StackModIO::getMotorSpeed(uint8_t motor_number)
{
    if (motor_number > 0 && motor_number < MAX_MODULES + 1)
    {
        return motors_reg[motor_number - 1];
    }
    else {
        return 0;
    }
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



void StackModIO::processPacket(char *packet)
{

    char        data[MAX_REGISTER_SIZE + 1];
    int         module_number = 0;
    bool        forward = true;
    int         val = 0;
    // Clear the packet status

    unsigned char packet_size = strlen(packet);

    if (packet_size < 5) return;  

    uint8_t address = packet[1]; // Get target address of packet
    uint8_t action = packet[2]; // Check if command or query

    // Identify which command was sent
    int cmd;
    for (cmd = 0; commands[cmd].cmd; cmd++)
    {
        if (!strncmp(&packet[3], commands[cmd].cmd, strlen(commands[cmd].cmd)))
            break;
    }

    // Address must match our address

    if (address != i2c_address) {
        //serial.println("INVALID ADDRESS");
        // return;
    }

    
    switch (cmd)
    {
    case MOTOR:

        // @A$MTR1+255

        // if (DEBUGGING)

        if (action == COMMAND && (strlen(packet) == commands[cmd].cmd_len)) {
            //int v = sscanf(&packet[7], "%d", &val);
            if (sscanf(&packet[6], "%d%d", &module_number, &val) == 2)
            {
                //serial.println(module_number);
              //  serial.println(val);
                setMotorSpeed(module_number, val);
            }
        }
        
        break;
    case ULTRASONIC:
        //if (DEBUGGING)
            //serial.println("This is an ultrasonic command");
        break;
    default:
        //serial.println("Command not found");
        break;
    }

    
}

int StackModIO::calculateChecksum(String packet)
{
    int sum = 0;
    int c = packet.length();
    for (int i = 0; i < c; i++) { sum += packet[i] - 32; }
    return (sum % 95) + 32;
}

