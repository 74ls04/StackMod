# StackMod: The Modular Robot Protocol
Intro description...
## Serial Message Format

**\<START>\<ADDRESS>\<COMMAND>\<DATA>\<END>\<CHECKSUM>**

* The START byte is ASCII "{" (123 decimal, 0x7B).
* The ADDRESS byte is any value from 64 to 95 decimal (0x40 to 0x5F, ASCII “@” to “_”).
* The ACTION byte is an ASCII “?” for GET and ASCII “$” for SET. 
* The COMMAND bytes are three characters.
* The DATA bytes are variable length and are described in the Data section.
* The END byte is ASCII "}" (125 decimal, 0x7D).
* The CHECKSUM is calculated by subtracting 32 from all the characters in the packet (excluding the checksum) and summing them. The modulo 95 of this value is then calculated and 32 is added back to that value. 

 
### Example Query and Response
> Query:    **{@?MTR2}X**
>Response:  **{@?MTR2+078}C**

## Available Commands
|Command|  Description|
|-------|-------------|
|ARM	| Arm / Disarm Motors  |
|MTR	| Motor       |
|SRV	| Servo	      |
|ULT	| Ultrasonic  |
|IRS	| IR          |
|DGT	| Digital Pin |
|ANL	| Analog Pin  |