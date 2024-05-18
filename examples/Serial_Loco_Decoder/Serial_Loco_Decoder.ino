#include "snail_dcc.h"

#define ADDRESS 3486 //1-10239

/*  Create a new Loco on your Command Station 
 *  with the following settings:
 *  Format: DCC
 *  Address: see above, 1-10239
 *  Speed steps: 14, 28, 126 (14 steps mode is compatible with 28 steps mode, Function F0 is interpreted as half-steps)
 *  Functions: F0-F12 
 */

//use precompiler magic for the address and stuff
#if ADDRESS<128
  #define ADDR1 ADDRESS
  #define INDEX_ADDR1   1                 //index of address byte in data packet
  #define INDEX_COMMAND 2                 //index of command in data packet
  #define INDEX_DATA    3                 //index of optional data in long data packets
#else
  #define ADDR1 ((ADDRESS>>8)|0b11000000) //0b11aaaaaa the upper 6 bits of the address
  #define ADDR2 (ADDRESS & 0b11111111)    //0baaaaaaaa the lower 8 bits of the address
  #define INDEX_ADDR1   1                 //index of  firt  address byte in data packet
  #define INDEX_ADDR2   2                 //index of second address byte in data packet
  #define INDEX_COMMAND 3                 //index of command in data packet
  #define INDEX_DATA    4                 //index of optional data in long data packets
  #define LONG_ADDRESS_MODE
#endif

Dcc dcc(2);

bool f[13] = {0};
bool dir = false;
unsigned char speed = 0;

void printState(){
  Serial.print("Loco ");
  Serial.print(ADDRESS);
  Serial.print(" Speed: ");
  Serial.print(speed);
  Serial.print(dir?" >":" <"); //check direction and show > or <
  if(f[0]) Serial.print(" FL");
  if(f[1]) Serial.print(" F1");
  if(f[2]) Serial.print(" F2");
  if(f[3]) Serial.print(" F3");
  if(f[4]) Serial.print(" F4");
  for(int i=5;i<13;i++) if(f[i]) {
    Serial.print(" F");Serial.print(i);
  }
  //example for directional lights
  if(f[0]&&dir)  Serial.print("  :-)");
  if(f[0]&&!dir) Serial.print("  (-:");
  Serial.println();
}

void setup() {
  dcc.begin();
  Serial.begin(115200);
}

void printbin(char x, char bits){
  for(int i=bits-1;i>=0;i--){
    Serial.print(x&(1<<i)?"1":"0");
  }
}

void loop() {
  //Read the next Packet from the DCC line.
  //byte0 length of the packet
  //byte1-n data packet, see S-9.2 https://dccwiki.com/NMRA/NMRA_Standards
  unsigned char* buf = dcc.nextPacket();

  //if it's not our address, do nothing (also ignore rest and broadcast packages...)
  if(buf[INDEX_ADDR1]!=ADDR1) return; 
  //also check 2nd address byte for long addresses
  #ifdef LONG_ADDRESS_MODE
    if(buf[INDEX_ADDR2]!=ADDR2) return;
  #endif

  //now we have the command
  unsigned char c = buf[INDEX_COMMAND];
  
  //is this a 28-step speed command? 
  if((c&0b11000000)==0b01000000){
    dir = c & 0b00100000;
    speed = ((c&0xf)<<1) | ((c&0x10)>>4); //01DSSSSS the lowest speed bit is stored in bit4
    if(speed<4) speed=0;
    speed = speed<<3; //now its between 0-255
  }
  
  //is this a 126-step speed command? 
  if((c&0b11111111)==0b00111111){
    unsigned char d = buf[INDEX_DATA];
    dir = d & 0b10000000;
    speed = d & 0x7f; //DSSSSSSS
    if(speed<2) speed=0;
    speed = speed<<1; //now its between 0-255
  }
  
  //is this a F0-F4 command? (Function Group one)
  if((c&0b11100000)==0b10000000){
    f[0] = c & 0b10000;
    f[4] = c & 0b01000;
    f[3] = c & 0b00100;
    f[2] = c & 0b00010;
    f[1] = c & 0b00001;
  }
  //is this a F5-F8 command? (Function Group two a)
  if((c&0b11110000)==0b10110000){
    f[8] = c & 0b1000;
    f[7] = c & 0b0100;
    f[6] = c & 0b0010;
    f[5] = c & 0b0001;
  }
  //is this a F9-F12 command? (Function Group two b)
  if((c&0b11110000)==0b10100000){
    f[12] = c & 0b1000;
    f[11] = c & 0b0100;
    f[10] = c & 0b0010;
    f[9]  = c & 0b0001;
  }

  //show something on the Serial Terminal
  printState();
}
