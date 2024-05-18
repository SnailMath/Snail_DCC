# Snail_DCC

This is a small library that I wrote in one day because there was no DCC library for the arduino that was working and easy to use.

## Warning: **NOT** NMRA compliant!

My goal was to receive the data that my model railroad controller sends so I could use that to control things like lights ect. I quickly found the [NMRA standard](https://dccwiki.com/NMRA/NMRA_Standards), where S-9.2 describes the DCC protocol.

Putting an optocoupler with an antiparallel diode (e.g. an LED to see if there's Power) onto the breadboard was really easy, but I think it would even be easier to use a very high resistor (way above 40k) to connect the DCC signal directly to the Arduino and abuse the safety diodes, but I didn't want to ruin my arduino, so I just have that idea for smaller ATtiny microcontrollers for a later day.

My library attaches an interrupt to the DCC pin (preferebly pin 2), and when it changes, the time is stored or the difference is calculated. If the bit is shorter than 80us, its probaby a `1` (58us) and if it's longer, it's probably a `0` (100us). While waiting for the next packet, my code just counts the preamble `1` bits, and when the `0` start bit is received, it switches to the next state and shifts the bits into a buffer. When the stop bit is received, my code switches the writing buffer and the reading buffer, so while slow main code is checking the packet, my code can already receive the next packet. 

The main code just consists of a call to my library to begin the DCC stuff, and the main loop where we get the next packet, and check what's inside. I strongly recommend you to read the NMRA Standard S-9.2 and S-9.2.1 to find out how the DCC protocol works.

## Examples:

The library comes with 4 examples, `Serial_Binary` is the first example I wrote, which just dumps every packet that it receives in binary format, this way it was easy to compare the data with my notes and the NMRA standard.  

The next example is `Serial_Info`, which just checks for every type of packet that I found useful and dumps that onto the serial port.  

The 3rd example is `Seria_Loco_Decoder`, which shows how a Loco decoder would work. You can edit the address in the code, and some precompiler magic turns it into either a short or a long address. Every packet to this address gets evaluated, speed in eiter 28 or 126 Steps is stored, 14 step-speed also works kinda because of compatibility with the 28 Steps mode. The speed steps are multiplied by a constant, so they fit into the range 0-255, because that looks good in the serial terminal. Functions F0-F4, F5-F8 and F9-12 are implemented, but I didn't implement everything above F12 because I just find the way that it's implemented kinda strange. As the name of the example implies, everything is just dumped onto the serial terminal.

The last example is `Acccessory_Decoder`. As the name implies, this is a working accessory decoder. You can type multiple addresses into the array at the beginning of the code, and the arduino will listen for those addresses and pulse it's ouptput pins if it receives a signal. For normal turnouts, you define 2 pins which could in theory connect via a big transistor to the solenoid of a turnout, and when you press red or green on the railroad controller, the specific solenoid is on only while the button is held down. For Lights you can configure a single pin to turn on with one button and off with another button. For a signal, you could connect the lamps to the arduino and configure it so one button turns on green and turns off red and vice versa, in the example there is even a configuration for a signal with a red lamp, a green lamp and a relay to switch the track's power, and another example for a signal with 3 aspects.

These examples are just to understand what my library does - or more like what it doesn't do, because all the decodnig of the DCC packets is left to the user, since this is not so complicated and it depends on which functions one wants to implement.

## Installation of this library

This is just an arduino library, go to your Home folder and find the folder called arduino, and go into the folder libraries. Download this archive and unpack it into a folder called Snail_Dcc. So the Folder Structure should look like this:

```
+ Documents
| + Arduino
| | + Libraries
| | | + Snail_Dcc
| | | | + examples
| | | | | + Accessory_Decoder
| | | | | | - Accessory_Decoder.ino
| | | | | + Serial_Binary
| | | | | | - Accessory_Decoder.ino
| | | | | + Serial_Info
| | | | | | - Serial_Info.ino
| | | | | + Serial_Loco_Decoder
| | | | | | - Serial_Loco_Decoder.ino
| | | | - README.MD
| | | | - snail_dcc.cpp
| | | | - snail_dcc.h
```

## Have Fun!
