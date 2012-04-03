/*
* Si4735 Serial Example Sketch
* Written by Ryan Owens for SparkFun Electronics
* Updated for the current state of the library by Radu - Eosif Mihailescu
*
* This example sketch allows a user to construct their own commands for the
* Si4735 and send them through the serial terminal.
* Learn more about the Si4735 commands in the Si4735 Programming Guide.
*
* HARDWARE SETUP:
* This sketch assumes you are using the Si4735 Shield from SparkFun
* Electronics.
* The shield should be plugged into an Arduino Main Board (Uno, Mega etc.)
*
* USING THE SKETCH:
* Once you've plugged the Si4735 Shield into your Arduino board, connect the
* Arduino to your computer and select the corresponding board and COM port
* from the Tools menu and upload the sketch. After the sketch has been
* updated, open the serial terminal using a 9600 bps baud speed. Using the 
* Si4735 Programming Guide as a reference, you can use the serial terminal to
* send commands to the Si4735. The command string must be terminated with
* either a newline or carriage return character. To enable this in the Arduino
* Serial Terminal, make sure the drop down menu located in the bottom right
* hand side of the terminal window (not the baud rate one, but right next to 
* that) reads 'Newline,' 'Carriage Return,' or 'Both NL and CR.'
* The sketch will convert the command string given by the user to the
* corresponding binary representation of the command and send it to the radio.
* The command string may only consist of hexadecimal characters ([0-9a-fA-F]).
* Available (case-insensitive) commands are:
*   r      - display response (long read)
*   s      - display status (short read)
*   x      - flush (empty) command string and start over
*   0-9a-f - compose command, at most 16 hex digits (forming an 8 byte
*            command) are accepted
*   CR/LF  - send command
*   ?      - display this list
* You can read more about the status and response strings in the Si4735
* datasheet.
*
* SAMPLE COMMANDS
* 11 - Power down the radio
* 015005 - Power up the radio in FM mode. (INT pin enabled, external 
*          oscillator used, analog outputs enabled)
* 015105 - Power up the radio in AM mode. (Same configuration as FM) 
*          NOTE: This same mode is used for both AM, short wave and long wave
*                reception.
* 20002602 - FM Tune command, tunes to 97.3 MHz (0x2602 = 9730 Decimal)
* 40000352 - AM Tune command, tunes to 850 KHz (0x352 = 850 Decimal)
* 1200340008FC - Set the lower band limit to 2.3 MHz (0x8FC = 2300 Decimal)
* 1200340159D8 - Set the upper band limit to 23 MHz (0x59D8 = 23000 Decimal)
*
* NOTES:
* This sketch will start the Si4735 in FM mode, feel free to subsequently
* send POWER_DOWN/POWER_UP commands to restart it in any other mode.
*/

//Due to a bug in Arduino, these need to be included here too/first
#include <SPI.h>
#include <Wire.h>

//Add the Si4735 Library to the sketch
#include <Si4735.h>

//Create an instance of the Si4735 named radio.
Si4735 radio;
byte response[16], status, command[8], numDigits;
char collectedDigits[17] = "", currentDigit, oneHexValue[5] = "0x";

void setup()
{
  //Create a serial connection
  Serial.begin(9600);

  //Initialize the radio
  radio.begin(SI4735_MODE_FM);
}

void loop()
{       
  //Wait to do something until a character is received on the serial port.
  if(Serial.available() > 0){
    //Copy the incoming character to a variable and fold case
    currentDigit = toupper(Serial.read());
    //Depending on the incoming character, decide what to do.
    switch(currentDigit){
      case 'R':
        //Get the latest response from the radio.
        radio.getResponse(response);
        //Print all 16 bytes in the response to the terminal.      
        Serial.print("Si4735 RSP");
        for(int i = 0; i < 4; i++) {
          if(i) Serial.print("           ");
          else Serial.print(" ");
          for(int j = 0; j < 4; j++) {
            Serial.print("0x");
            Serial.print(response[i * 4 + j], HEX);
            Serial.print(" [");
            Serial.print(response[i * 4 + j], BIN);
            Serial.print("]");
            if(j != 3) Serial.print(", ");
            else
              if(i != 3) Serial.print(",");
          };
          Serial.println("");
        };
        break;
      case 'S':
        status = radio.getStatus();
        Serial.print("Si4735 STS 0x");
        Serial.print(status, HEX);
        Serial.print(" [");
        Serial.print(status, BIN);
        Serial.println("]");
        break;
      case 'X':
        collectedDigits[0] = '\0';
        Serial.println("Command string truncated, start over.");
        break;
      //If we get a LF or CR character, send the command to the radio.
      case '\n':
      case '\r':
        numDigits = strlen(collectedDigits);
        //Silently ignore empty lines, this also gives us CR & LF support
        if(numDigits) {
          if(numDigits % 2){    
            memset(command, 0x00, 8);
            for(int i = 0; i++; i < (numDigits / 2)) {
              strncpy(&oneHexValue[2], &collectedDigits[i * 2], 2);
              command[i] = strtoul(oneHexValue, NULL, 16);
            }
            //End command string echo
            Serial.println("");
            //Send the current command to the radio.
            radio.sendCommand(command[0], command[1], command[2],
                              command[3], command[4], command[5],
                              command[6], command[7]);
          } else Serial.println("Odd number of hex digits, need even!");
        } 
        break;
      case '?':
        Serial.println("Available commands:");
        Serial.println("* r      - display response (long read)");
        Serial.println("* s      - display status (short read)");
        Serial.println("* x      - flush (empty) command string and start over");
        Serial.println("* 0-9a-f - compose command, at most 16 hex digits (forming an 8 byte");
        Serial.println("           command) are accepted");
        Serial.println("* CR/LF  - send command");
        Serial.println("* ?      - display this list");
        break;
      //If we get any other character and it's a valid hexidecimal character,
      //copy it to the command string.
      default: 
        if((currentDigit >= '0' && currentDigit <= '9') || 
           (currentDigit >= 'A' && currentDigit <= 'F'))
          if(strlen(collectedDigits) < 16) {
            strncat(collectedDigits, &currentDigit, 1);
            //Echo the command string as it is composed
            Serial.print(currentDigit);
          } else Serial.println("Too many hex digits, 16 maximum!");
        else Serial.println("Invalid command!");
        break;
    }
  }   
}
