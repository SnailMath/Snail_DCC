#include "snail_dcc.h"

Dcc dcc(2);

void setup() {
  dcc.begin();
  Serial.begin(115200);
}

void printbin(char x, char bits){
  for(int i=bits-1;i>=0;i--){
    Serial.print(x&(1<<i)?"1":"0");
  }
}

void loco_info(char* buf){
  if((buf[2]&0b11000000)==0b01000000){
      Serial.print(", Speed: ");
      int speed = (buf[2]&0x0f)<<1; //old speed format
      speed |= (buf[2]&0x10)>>4;     //new lowest bit
      Serial.print(speed); //1 is emergency stop
      Serial.print(buf[2]&0x20?" >":" <");
    }else if((buf[2]&0b11100000)==0b10000000){
      Serial.print(", Lamps: ");
      Serial.print(buf[2]&0b00010000?"on":"off");
      Serial.print(", F4-F1: ");
      printbin(buf[2],4);
    }else if((buf[2]&0b11100000)==0b10100000){
      if(buf[2]&0b00010000) Serial.print(", F8-F5: ");
      else Serial.print(", F12-F9: ");
      printbin(buf[2],4);
    }else if(buf[2]==0b00111111){
      Serial.print(", Speed: ");
      Serial.print(buf[3]&0x7F); //1 is emergency stop
      Serial.print(buf[3]&0x80?" >":" <");
    }else{
      Serial.print(" Don't know: ");
      printbin(buf[2],8);
    }
    Serial.println();
}

void loop() {
  //Read the next Packet from the DCC line.
  //byte0 length of the packet
  //byte1-n data packet, see S-9.2 https://dccwiki.com/NMRA/NMRA_Standards
  unsigned char* buf = dcc.nextPacket();
  if(buf[1]==0){
    //Reset command
    Serial.print("Reset ");
    if (buf[0]>1) printbin(buf[2],8);
    Serial.println();
  }else if(buf[1]<=127){
    //Short Address Loco
    Serial.print("Loco ");
    Serial.print(buf[1],DEC);
    loco_info(buf);
  }else if(buf[1]<=191){
    //Accessory Decoder
    Serial.print("Accessory ");
    int addr = buf[1]&0b00111111;
    addr |= (~buf[2]<<2)&0b0111000000;
    int port = ((buf[2]&0b110)>>1)+1;
    Serial.print(addr);
    Serial.print(":");
    Serial.print(port);
    Serial.print(" (");
    Serial.print((addr-1)*4+port);
    Serial.print(") ");
    Serial.print(buf[2]&0b1?"g ":"r ");
    Serial.print(buf[2]&0b1000?"on":"off");
    if(~buf[2]&0b10000000){
      Serial.print(" val: "); Serial.print(buf[3]);
    }
    Serial.println();
  
  }else if(buf[1]<=231){
    //Short Address Loco
    Serial.print("Loco long ");
    int addr = (buf[1]&0b00111111)<<8;
    addr |= buf[2];
    Serial.print(addr);
    loco_info(buf+1); //just a small hack to reuse the code for short addresses
  }else{
    for(int i=1;i<=buf[0];i++){
      Serial.print(int(buf[i]));
    }
    Serial.println();
  }
    
}
