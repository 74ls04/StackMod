# ModBot: The Modular Robot Protocol
Intro description...
## Serial Message Format

**\<START>\<ADDRESS>\<COMMAND>\<DATA>\<END>\<CHECKSUM>**

* The Start byte is ASCII "{" (123 decimal, 0x7B).
* The Address byte is any value from 64 to 95 decimal (0x40 to 0x5F, ASCII “@” to “_”).
* Commands are three characters preceded by an ASCII “?” or “$.” 
  *  Query Commands are preceded by “?” and SET commands are preceded by “$”.
 * The Data bytes are variable length and are described below
 * The End byte is ASCII "}" (125 decimal, 0x7D).
 
### Example Query and Response
> Query:    **{@?MTR}**
>Response:  **{@?MTRaAjdEsfa}u**

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