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
//Every Arduino board will work with this sketch.
//XBlast IOs are 3.3V but 5V-tolerant.

//Connect GND between Arduino and XBlast
//Connect Arduino pin 10(SS) to XBlast OUT3
//Connect Arduino pin 11(MOSI) to XBlast OUT2
//Connect Arduino pin 13(SCK) to XBlast OUT1
//Leave Arduino pin 12(MISO) unconnected.

#include <SPI.h>

#define ringBufferSize 800 // To accommodate ATMega168 devices with 1KB SRAM
unsigned char buf[ringBufferSize];

unsigned short dataInPos = 0;
unsigned short dataOutPos = 0;
bool ringBufRollOver = false;

void setup (void)
{
  Serial.begin(250000);   // print on terminal
  Serial.println("XBlast SPI debugger");
  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);
  
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  
  // turn on SPI in slave mode
  SPCR |= _BV(SPE);

  // now turn on interrupts
  SPI.attachInterrupt();

}  // end of setup


// SPI interrupt routine
ISR (SPI_STC_vect)
{
    if(ringBufRollOver == false || dataInPos < dataOutPos)
    {
        // Drop byte if ring buffer is full
	    buf[dataInPos++] = SPDR;
	    
	    if(dataInPos >= ringBufferSize)
	    {
	        dataInPos = 0;
	        ringBufRollOver = true;
	    }
    }
}

// main loop - wait for flag set in interrupt routine
void loop (void)
{
    while(1)
    {
        if((dataOutPos != dataInPos) || (dataOutPos == 0 && dataInPos == 0 && ringBufRollOver == true))
        {
            Serial.write(buf[dataOutPos++]);   
	    	if(dataOutPos >= ringBufferSize)
	    	{
	    	    ringBufRollOver = false;
	    	}
        } 
    }
}

