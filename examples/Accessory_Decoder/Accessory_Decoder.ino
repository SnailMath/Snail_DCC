#include "snail_dcc.h"

#define red *2
#define green *2+1

// each accessory can either just be an impulse, 
// like the wire for a switch, 
// or continuous, for example lamps.

// Example 1:
// Accessory 1 is a turnout, that's controlled by two solenoids.
// The solenoids are connected via a strong transistor to pins 11 and 12.
// Pressing the buttons energizs the coils only while the button is pressed.
// For this standard behaviour, put the address in the first column, 
// the pin number in the second, and -1 in the third.
// Remember to use one address with pin for green and one for red.

// Example 2:
// Accessory 2 should control the arduino default lamp which is dierctly next to pin13.
// The green button only turns the lamp on, the red button only off.
// For this special behaviour, put the address in the first column, 
// the pin number in the second, and 0 or 1 in the third column, 
// depending if the button should turn the output just on or just off
// Remember to use one adderss for on and one for off.

//Example 3:
// Accessory 3 should be a signal with a red light on pin 10 and a green light on pin 9. 
// In addition to that, a relay which is connected to pin 7+8 (through transistors) will interrupt power to the tracks.
// For this, we need three entries for each button, 
// the red button turns on the red light, off the green light, and pulses one relay contact, 
// the green button turns off the red light, on the green light, and it pulses the other relay contact.

// Example 4:
// A german railway signal with red, green and yellow lights is connected to pins 4,5,6 respectively. 
// It's controlled by addresses 5 and 6, the address 5 button red turns the signal to red, addr 5 button green turns it to green, 
// and the green button on address 6 turns it to green-yellow.

int accessory[] = {
  //aka decoder 1 outputs 1-4
  //Example 1
  1 red,    11, -1,
  1 green,  12, -1,

  //Example 2
  2 red,    13,  0, // 13 will be turned off with the red button
  2 green,  13,  1, // 13 will be turned on with the green button

  //Example 3
  3 red,    10,  1, // turn on red light when red is pressed
  3 red,     9,  0, // turn off green light when red is pressed
  3 red,     8, -1, // pulse red relay contact to turn off power when red is pressed
  3 green,  10,  0, // turn off red light when green is pressed
  3 green,   9,  1, // turn on green light when green is pressed
  3 green,   7, -1, // pulse green relay contact to turn on power when green is pressed
  
  //4 red,    -1, -1, //the addresses don't need to be continuous
  //4 green,  -1, -1,
  
  //aka decoder 2 outputs 1-4
  //Example 4
  5 red,     4,  1, // only red
  5 red,     5,  0,
  5 red,     6,  0,
  5 green,   4,  0,
  5 green,   5,  1, // only green
  5 green,   6,  0,
  6 green,   4,  0,
  6 green,   5,  1, // both green and yellow
  6 green,   6,  1,

  //6 red,    -1, -1, //this could be a simple uncoupling track
  
  //7 red,    -1, -1,
  //7 green,  -1, -1,
  
  //8 red,    -1, -1,
  //8 green,  -1, -1,
};
unsigned long len = sizeof(accessory)/sizeof(accessory)[0];

Dcc dcc(2);

void setup() {
  dcc.begin();
  Serial.begin(115200);
  for(int i=0; i<len; i+=3){
    pinMode(accessory[i+1],OUTPUT);
  }
}

void loop() {
  //Read the next Packet from the DCC line.
  //byte0 length of the packet
  //byte1-n data packet, see S-9.2 https://dccwiki.com/NMRA/NMRA_Standards
  unsigned char* buf = dcc.nextPacket();

  if ((buf[1]&0b11000000)!=0b10000000) return; //break if it's not in the form of 0b10AAAAAA, so no accessory address.
  if ((buf[2]&0b10000000)!=0b10000000) return; //break if it's not basic accessory
  //convert "10AAAAAA 1XXXCDDD" into "XXXAAAAAADDD"
  int addr = (int(~buf[2])<<5)&0b111000000000; //take the top 3 bits which are inverted
  addr    |= (int( buf[1])<<3)&0b000111111000; //take the middle 6 bits
  addr    |= (int( buf[2])   )&0b000000000111; //take the lower 3 bits, which are actually the port number, tecnically not the address, but we don't care.
  addr    -= 6; //just because we start counting the addresses differently...
  bool state = buf[2]&0b00001000; //is the port turned on or off?
  
  Serial.print(addr/2); //divide by 2 so the red and green button of the same accessory have the same address again.
  Serial.print(addr%2==0?" red":" green"); //and show if it was red or green seperately.
  Serial.println(state?" on":" off");

  //now turn on or off the actual pins on the arduino
  //iterate over every config
  for(int i=0; i<len; i+=3){
    //if the address matches AND a pin is defined
    if(addr==accessory[i] && accessory[i+1]!=-1){
      if (accessory[i+2]==-1){
        //if this is a normal output, just turn it on or off, depending if the button was pressed or released
        digitalWrite(accessory[i+1], state);
      }else{
        //if this is a special output, turn it either just on, or just off.
        digitalWrite(accessory[i+1],accessory[i+2]);
      }
    }
  }
}
