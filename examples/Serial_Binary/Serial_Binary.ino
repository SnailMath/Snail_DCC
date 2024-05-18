#include "snail_dcc.h"

Dcc dcc(2);

void setup() {
  dcc.begin();
  Serial.begin(115200);
}

void printbin(char x, char end){
  for(int i=7;i>=0;i--){
    Serial.print(x&(1<<i)?"1":"0");
  }
  Serial.print(end);
}

void loop() {
  //Read the next Packet from the DCC line.
  //byte0 length of the packet
  //byte1-n data packet, see S-9.2 https://dccwiki.com/NMRA/NMRA_Standards
  char* buf = dcc.nextPacket();
  for(int i=0;i<buf[0];i++){
    printbin(buf[i+1],' ');
  }
  Serial.println();
}
