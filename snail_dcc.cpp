#include "Arduino.h"
#include "snail_dcc.h"

//#define TEST 3 //test if the arduino is fast enough by mirroring the DCC signal on another pin

//fast functions only work for uno pin 0-7 (PORT D)
#define fastRead(p) PIND&(1<<p)
#define fastToggle(p) PORTD^=(1<<p);


    static int Dcc::_pin; 
    static unsigned int Dcc::preamble;// = 0;
    static char Dcc::buf[NRBUF][BUFSIZE];
    static unsigned int Dcc::state;// = 0;
    static volatile int Dcc::buf_w;// = 0;
    static int Dcc::buf_r;// = NRBUF-1;
    static bool Dcc::dcc_toggle;  
    static char Dcc::bitcounter;// = 0;
    static unsigned long Dcc::last;// = 0;



Dcc::Dcc(int pin){
  _pin = pin;
  preamble = 0;
  state = 0;
  buf_w = 0;
  buf_r = NRBUF-1;
  bitcounter = 0;
}

//check if a new package is ready
bool Dcc::available(){
  return (buf_r+1)%NRBUF!=buf_w;
}

char* Dcc::nextPacket(){
  //wait until the next packet is ready
  while (!available());
  
  //move our read pointer to this current packet, so the last one can be overwritten
  buf_r++; 
  if(buf_r==NRBUF)buf_r=0;

  //return the pointer to this buffer
  return buf[buf_r];
  
}

void Dcc::begin(){
  pinMode(_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(_pin), handleDCC, CHANGE);
  #ifdef TEST
    pinMode(TEST, OUTPUT);
  #endif
}

void Dcc::handleDCC() {
  #ifdef TEST
    fastToggle(TEST)
  #endif
  
  unsigned long now = micros();
  //if (fastRead(_pin)){
  if (dcc_toggle^=1){
    last = now;
    return;
  }
  
  //check if our buffer is full and we need to ignore everything
  if (buf_w==buf_r)return;

  //read if it's a 1 or a 0
  bool bit = (now - last) < 80;
  
  if (state == 0) { //preamble, wait for packet start
    if (bit) {
      preamble++;
    } else {
      if (preamble >= 10) {
        state++;
      }
      preamble = 0;
    }
  } else if (state & 1) { //read current byte
    int i = (state >> 1) + 1;
    buf[buf_w][i] = (buf[buf_w][i] << 1) | bit;
    if (++bitcounter == 8) {
      bitcounter = 0;
      state++;
    }
  } else { //check if data start bit or packet end bit
    if (bit) {
      //the packet is finished
      //done = 1;
      buf[buf_w][0]=(state>>1);
      state = 0;
      if(++buf_w==NRBUF)buf_w=0;
      //ignore = 1;
    } else {
      //there's more data to receive
      state++;
    }
  }
}
