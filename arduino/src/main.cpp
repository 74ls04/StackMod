#include "Arduino.h"
#include <EEPROM.h>
#include <Wire.h>

#define  CTRL_REG_SIZE 4
#define  I2C_ADDRESS 0x38

/**** EEPROM Address ****/
#define RAMP_ADDR 0
#define SLOW_ADDR 1
#define MED_ADDR 2
#define FAST_ADDR 3

/**** Motor padding values for RPM matching ****/
#define  M1_PAD 0
#define  M2_PAD 0
#define  M3_PAD 0
#define  M4_PAD 0

/**** Command Register Indices ****/
#define  CMD 0
#define  SPEED 1
#define  DIR 2
#define  RATIO 3

/**** Pin Definitions ****/
#define  LF_1 16	//M1
#define  LF_2 17    //M1
#define  LB_1 14    //M2
#define  LB_2 15    //M2
#define  RF_1 7     //M3
#define  RF_2 8     //M3
#define  RB_1 12    //M4
#define  RB_2 13    //M4

#define  M1 3	//M1 PWM
#define  M2 9	//M2 PWM
#define  M3 10	//M3 PWM
#define  M4 11	//M4 PWM

/**** Enums ****/
enum Command_list {
    MOTOR = 77
};

enum Motor_cmd {
	FORWARD = 70,
	REVERSE = 71,
	BACK = 66,
	TURN = 84,
	STOP = 83,
	LEFT = 76,
	RIGHT = 82,
	VEER = 86,
	DRIVE = 68,
	CONFIG = 67,
	ENABLE = 72,
	DISABLE = 73,
	ENABLED = 48,
	DISABLED = 49
};

/**** Prototypes ****/
void set_dir(Motor_cmd motor, Motor_cmd direction);
void forward_dir();
void back_dir();
void turn_left_dir();
void turn_right_dir();
void set_brake();
void drive();
void ramp_motors(int target_left, int target_right, int output_left, int output_right);

void parse_cmd(int command);
void store_settings(int ramp, int slowSetpoint, int medSetpoint, int fastSetpoint);
void read_settings();
void configure_pins();

int calculate_checksum(String packet);
void process_packet();

/**** Control Register ****/
char controlReg[CTRL_REG_SIZE];
int motorSpeeds[4];

char *regP = controlReg;
bool firstByte = true;

//Persistent config variables
int ramp = 1;
int slowSetpoint;
int medSetpoint;
int fastSetpoint;
boolean NEWSETPOINT = false;
// Serial debugging
int incomingByte = 0;   // for incoming serial data

boolean NEWPACKET = false;
const byte numChars = 32;
char packet_buffer[numChars];

boolean MOTORSENABLED = false;




void setup() {
    
    configure_pins();

    // Change PWM frequency to 31250 
    // https://playground.arduino.cc/Main/TimerPWMCheatsheet

	TCCR1B = TCCR1B & 0b11111000 | 0x01;
	TCCR2B = TCCR2B & 0b11111000 | 0x01;

	read_settings();

    memset(motorSpeeds, 0, sizeof(motorSpeeds)); // Reset speed values

	Wire.begin(I2C_ADDRESS);                // join i2c bus with address 0x30
	Wire.onReceive(receive_i2c_packet); // register event
	Serial.begin(9600);           // start serial for output
}

void configure_pins() {
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

void receive_serial_packet() {
    static boolean receiving = false;
    static byte index = 0;
    char startMarker = '{';
    char endMarker = '}';
    char inByte;

// if (Serial.available() > 0) {
    
    while (Serial.available() > 0 && NEWPACKET == false) {

        inByte = Serial.read();

        if (receiving == true && inByte != startMarker) {
            if (inByte != endMarker) {
                packet_buffer[index] = inByte;
                index++;
                if (index >= numChars) {
                    index = numChars - 1;
                }
            } else {
                packet_buffer[index] = '\0'; // terminate the string
                receiving = false;
                index = 0;
                NEWPACKET = true;
            }
        } else if (inByte == startMarker) {
            receiving = true;
            memset(packet_buffer, 0, sizeof(packet_buffer)); // Empty char buffer
        }
    }
}

void receive_i2c_packet(int numBytes) {
    static boolean receiving = false;
    static byte index = 0;
    char startMarker = '{';
    char endMarker = '}';
    char inByte;

// if (Serial.available() > 0) {
    
    while (Wire.available() > 0 && NEWPACKET == false) {

        inByte = Wire.read();

        if (receiving == true && inByte != startMarker) {
            if (inByte != endMarker) {
                packet_buffer[index] = inByte;
                index++;
                if (index >= numChars) {
                    index = numChars - 1;
                }
            } else {
                packet_buffer[index] = '\0'; // terminate the string
                receiving = false;
                index = 0;
                NEWPACKET = true;
            }
        } else if (inByte == startMarker) {
            receiving = true;
            memset(packet_buffer, 0, sizeof(packet_buffer)); // Empty char buffer
        }
    }
}


void loop() {
    receive_serial_packet();
    process_packet();

	delay(50);
}


int calculate_checksum(String packet) {
    int sum = 0;
    int c = packet.length();
    for (int i = 0; i < c; i++) { sum += packet[i] - 32;}
    return (sum % 95) + 32;
}

int process_motor_speed(int left_speed, int right_speed) {
	
	if (!MOTORSENABLED) {
		Serial.println("Motors are disabled!");
		digitalWrite(M1, LOW);
		digitalWrite(M2, LOW);
		digitalWrite(M3, LOW);
		digitalWrite(M4, LOW);
		return 1;
	}

	NEWSETPOINT = true;

	int left_adjusted_speed = left_speed - 50;
	int right_adjusted_speed = right_speed - 50;
	int left_current_output = motorSpeeds[0];
	int right_current_output = motorSpeeds[2];

	if (left_adjusted_speed < 0) {
		set_dir(LEFT, REVERSE);
		Serial.println("Reverse");
	}

	if (right_adjusted_speed < 0) {
		set_dir(RIGHT, REVERSE);
		Serial.println("Reverse");
	}

	if (left_adjusted_speed == 0) {
		set_dir(LEFT, STOP);
	}

	if (right_adjusted_speed == 0) {
		set_dir(RIGHT, STOP);
	}

	if (left_adjusted_speed > 0) {
		Serial.println("Forward");
		set_dir(LEFT, FORWARD);
	}

	if (right_adjusted_speed > 0) {
		Serial.println("Forward");
		set_dir(RIGHT, FORWARD);
	}

	ramp_motors(abs(left_adjusted_speed), abs(right_adjusted_speed), left_current_output, right_current_output);

}

void process_packet() {
    if (NEWPACKET == true) {
        NEWPACKET = false;
        Serial.println(packet_buffer);
        static byte packet_size = strlen(packet_buffer);

        if (packet_size < 4) return;

        byte address = packet_buffer[0];
        char action = packet_buffer[2];
        char command = packet_buffer[3];
        
        // Address must match our address
        if (address != I2C_ADDRESS) { 
            Serial.println("INVALID ADDRESS");
            return;
        }

        // Determine if this is a command or query packet
        if (action == 'c') {
            switch(command) {
                case MOTOR:
                    //8HcMdd
                    if (packet_size == 6) {
						process_motor_speed((int)packet_buffer[4], (int)packet_buffer[5]);
                    } else {
                        Serial.println("Invalid packet size for motor command");
                        return;
					}
				case ENABLE:	
					//8HcE1
					if (packet_size == 5) {
						if (packet_buffer[4] == ENABLED) {
							MOTORSENABLED = true;
						} else if (packet_buffer[4] == DISABLED) {
							MOTORSENABLED = false;
						}
					} else {
                        Serial.println("Invalid packet size for enable command");
                        return;						
					}

                default:
                    break;
            }
        } else if (action == '?') {
            Serial.println("This is a query");
        }
        
        // Serial.print(motorSpeeds[0]);
        // Serial.print(motorSpeeds[1]);
        // Serial.print(motorSpeeds[2]);
        // Serial.print(motorSpeeds[3]);
        /*
        for (int i = 0; i < packet_size; i++) {
            
        } */
    }
}

/**
 * 
 * Ramp up / down our motors at a fixed rate
 * 
 **/
void ramp_motors(int target_left, int target_right, int output_left, int output_right) {
	int rate = 1;
	NEWSETPOINT = false;
	while (output_left != target_left || output_right != target_right) {
		if ((target_left - output_left) > rate) {
			output_left += rate;
			motorSpeeds[0] = output_left - M1_PAD >= 0 ? output_left - M1_PAD : 0;
			motorSpeeds[1] = output_left - M2_PAD >= 0 ? output_left - M2_PAD : 0;
		}  else if ((output_left - target_left) > rate) {
			output_left -= rate;
			motorSpeeds[0] = output_left - M1_PAD >= 0 ? output_left - M1_PAD : 0;
			motorSpeeds[1] = output_left - M2_PAD >= 0 ? output_left - M2_PAD : 0;
		} else {
			output_left =target_left;
			motorSpeeds[0] = output_left - M1_PAD >= 0 ? output_left - M1_PAD : 0;
			motorSpeeds[1] = output_left - M2_PAD >= 0 ? output_left - M2_PAD : 0;
		}
		
		if ((target_right - output_right) > rate) {
			output_right += rate;
			motorSpeeds[2] = output_right - M3_PAD >= 0 ? output_right - M3_PAD : 0;
			motorSpeeds[3] = output_right - M4_PAD >= 0 ? output_right - M4_PAD : 0;
		} else if ((output_right - target_right) > rate) {
			output_right -= rate;
			motorSpeeds[2] = output_right - M3_PAD >= 0 ? output_right - M3_PAD : 0;
			motorSpeeds[3] = output_right - M4_PAD >= 0 ? output_right - M4_PAD : 0;
		} else {
			output_right = target_right;
			motorSpeeds[2] = output_right - M3_PAD >= 0 ? output_right - M3_PAD : 0;
			motorSpeeds[3] = output_right - M4_PAD >= 0 ? output_right - M4_PAD : 0;
		}

		drive();
		
		if (NEWSETPOINT) break;

		Serial.print(map(output_left, 0, 50, 150, 255));
		Serial.print(" | ");
		Serial.println(map(output_right, 0, 50, 150, 255));
		delay(20);

		
	}

}

void drive() {
	analogWrite(M1, map(motorSpeeds[0], 0, 50, 150, 255));
	analogWrite(M2, map(motorSpeeds[1], 0, 50, 150, 255));
	analogWrite(M3, map(motorSpeeds[2], 0, 50, 150, 255));
	analogWrite(M4, map(motorSpeeds[3], 0, 50, 150, 255));
}

void store_settings(int ramp, int slowSetpoint, int medSetpoint, int fastSetpoint) {
	EEPROM.write(RAMP_ADDR, ramp);
	EEPROM.write(SLOW_ADDR, slowSetpoint);
	EEPROM.write(MED_ADDR, medSetpoint);
	EEPROM.write(FAST_ADDR, fastSetpoint);

	read_settings();
}

void read_settings() {
	ramp = EEPROM.read(RAMP_ADDR);
	slowSetpoint = EEPROM.read(RAMP_ADDR);
	medSetpoint = EEPROM.read(RAMP_ADDR);
	fastSetpoint = EEPROM.read(RAMP_ADDR);
};

/**
 * Autonomous mode
 **/

/*
void parse_cmd(int command) {
	switch (command) {
		case FORWARD:
	);
			b_dirreak;

		case BACK:
			back_dir();
			break;

		case TURN:
			if (controlReg[DIR] == LEFT) {
	();
			}
_dir			if (controlReg[DIR] == RIGHT) {
				turn_right_dir();
			}
			break;
		case VEER:
	);
			i_dirf (controlReg[DIR] == LEFT) {
	();
			}
_dir			if (controlReg[DIR] == RIGHT) {
				turn_right_dir();
			}
			break;
		case STOP:
			set_brake();
			break;

		default:
			Serial.println("No Command");
			break;
		}
}
*/


void set_dir(Motor_cmd motor, Motor_cmd direction) {

	switch (motor) {
		case LEFT:
			if (direction == FORWARD) {
					digitalWrite(LB_1, 0);
					digitalWrite(LB_2, 1);
					digitalWrite(LF_1, 0);
					digitalWrite(LF_2, 1);
			} else if (direction == REVERSE) {
					digitalWrite(LB_1, 1);
					digitalWrite(LB_2, 0);
					digitalWrite(LF_1, 1);
					digitalWrite(LF_2, 0);				
			} else {
					digitalWrite(LB_1, 0);
					digitalWrite(LB_2, 0);
					digitalWrite(LF_1, 0);
					digitalWrite(LF_2, 0);
			}
		case RIGHT:
			if (direction == FORWARD) {
					digitalWrite(RB_1, 1);
					digitalWrite(RB_2, 0);
					digitalWrite(RF_1, 1);
					digitalWrite(RF_2, 0);
			} else if (direction == REVERSE) {
					digitalWrite(RB_1, 0);
					digitalWrite(RB_2, 1);
					digitalWrite(RF_1, 0);
					digitalWrite(RF_2, 1);			
			} else {
					digitalWrite(LB_1, 0);
					digitalWrite(LB_2, 0);
					digitalWrite(LF_1, 0);
					digitalWrite(LF_2, 0);
			}
		default:
			break;
	}
}

void forward_dir() {
	digitalWrite(LB_1, 0);
	digitalWrite(LB_2, 1);
	digitalWrite(LF_1, 0);
	digitalWrite(LF_2, 1);

	digitalWrite(RB_1, 1);
	digitalWrite(RB_2, 0);
	digitalWrite(RF_1, 1);
	digitalWrite(RF_2, 0);
}

void back_dir() {
	digitalWrite(LB_1, 1);
	digitalWrite(LB_2, 0);
	digitalWrite(LF_1, 1);
	digitalWrite(LF_2, 0);

	digitalWrite(RB_1, 0);
	digitalWrite(RB_2, 1);
	digitalWrite(RF_1, 0);
	digitalWrite(RF_2, 1);
}

void turn_left_dir() {
	digitalWrite(LB_1, 1);
	digitalWrite(LB_2, 0);
	digitalWrite(LF_1, 1);
	digitalWrite(LF_2, 0);

	digitalWrite(RB_1, 1);
	digitalWrite(RB_2, 0);
	digitalWrite(RF_1, 1);
	digitalWrite(RF_2, 0);
}

void turn_right_dir() {
	digitalWrite(LB_1, 0);
	digitalWrite(LB_2, 1);
	digitalWrite(LF_1, 0);
	digitalWrite(LF_2, 1);

	digitalWrite(RB_1, 0);
	digitalWrite(RB_2, 1);
	digitalWrite(RF_1, 0);
	digitalWrite(RF_2, 1);
}

void set_brake() {
	digitalWrite(LB_1, 1);
	digitalWrite(LB_2, 1);
	digitalWrite(LF_1, 1);
	digitalWrite(LF_2, 1);

	digitalWrite(RB_1, 1);
	digitalWrite(RB_2, 1);
	digitalWrite(RF_1, 1);
	digitalWrite(RF_2, 1);
}