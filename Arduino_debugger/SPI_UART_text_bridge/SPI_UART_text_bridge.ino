/* 
XBlast SPI debug string reader
Copyright (C) 2016  Benjamin Fiset-Deschênes
contact@benjaminfd.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. 
*/

//Small sketch to read debug data coming from XBlast OS
//Through the GPO ports of XBlast modchip.
//XBlast sends data via bit-banged SPI at max 166KHz, MSB first, 
//data latched on rising edge and idle clock to low.
//Arduino relay SPI-in data to UART-out. This way, text can be
//picked up in a terminal on your computer.
//Every Arduino board will work with this sketch but prefer one
//that runs on 3.3V instead of the more common 5V.

//Connect GND between Arduino and XBlast
//Connect Arduino pin 10(SS) to XBlast OUT3
//Connect Arduino pin 11(MOSI) to XBlast OUT2
//Connect Arduino pin 13(SCK) to XBlast OUT1
//Leave Arduino pin 12(MISO) unconnected.

#include <SPI.h>

char buf;
//volatile byte posCurrentLine;
//No circular buffer for now.
//volatile byte posSPIin;
//volatile byte posSerialout;
//volatile boolean sendNewLine;
volatile boolean sendCharacter;

void setup (void)
{
  Serial.begin(115200);   // print on terminal
  Serial.println("XBlast SPI debugger");
  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);
  
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  
  // turn on SPI in slave mode
  SPCR |= _BV(SPE);
  
  // get ready for an interrupt 
  //posCurrentLine = 0;   // buffer empty
  //posSPIin = 0;
  //posSerialout = 0;
  //sendNewLine = false;
  sendCharacter = false;

  // now turn on interrupts
  SPI.attachInterrupt();

}  // end of setup


// SPI interrupt routine
ISR (SPI_STC_vect)
{
    buf = SPDR;  // grab byte from SPI Data Register
    sendCharacter = true;
}

// main loop - wait for flag set in interrupt routine
void loop (void)
{
  if (sendCharacter)
  {
    Serial.print(buf);
    if(buf == '\0'){
        Serial.println();
    }
    sendCharacter = false;
  }
    
}

