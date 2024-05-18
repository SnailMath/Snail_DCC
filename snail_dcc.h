#ifndef SNAIL_DCC_H
#define SNAIL_DCC_H

#include "Arduino.h"

#define BUFSIZE 10
#define NRBUF   2
    
class Dcc
{
  public:
    Dcc(int pin); //create our DCC objekt 
    void begin(); //start to listen for dcc commands
    char* nextPacket(); //get the a char* to our next package in the following format:
    //{length, first_byte, second_byte, third_byte, ... , last_byte}
    bool available(); //check if a new package is ready
    
  private:
    static int _pin; 
    static unsigned int preamble;// = 0;
    static char buf[NRBUF][BUFSIZE];
    static unsigned int state;// = 0;
    /*  000 - preamble, wait for packet start
        001 - read 0th byte
        010 - check start bit
        011 - read 1th byte
        100 - check start bit (or packet end bit)
        101 - read 2nd byte
        ...
    */
    //the current buffer where we're writing to
    static volatile int buf_w;// = 0;
    //the current buffer where we're reading from
    static int buf_r;// = NRBUF-1;
    static bool dcc_toggle;  
    static char bitcounter;// = 0;
    static unsigned long last;// = 0;
    static void handleDCC();
};
#endif
