//Small sketch to read debug data coming from XBlast OS
//Through the GPO ports of XBlast modchip.
//XBlast sends data via bit-banged SPI at 166KHz, MSB first, 
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

char buf [200];
volatile byte pos;
volatile boolean process_it;

void setup (void)
{
  Serial.begin (115200);   // print on terminal

  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);
  
  // turn on SPI in slave mode
  SPCR |= _BV(SPE);
  
  // get ready for an interrupt 
  pos = 0;   // buffer empty
  process_it = false;

  // now turn on interrupts
  SPI.attachInterrupt();

}  // end of setup


// SPI interrupt routine
ISR (SPI_STC_vect)
{
byte c = SPDR;  // grab byte from SPI Data Register
  
  // add to buffer if room
  if (pos < sizeof buf)
    {
    buf [pos++] = Bit_Reverse(c);  //SPI is MSB first and UART is LSB first
    
    if (c == '\n' || c == '\0' || pos == sizeof buf)
      process_it = true;
      
    }
}

// Reverse the order of bits in a byte. 
// I.e. MSB is swapped with LSB, etc. 
unsigned char Bit_Reverse( unsigned char x ) 
{ 
    x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa); 
    x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc); 
    x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0); 
    return x;    
} 

// main loop - wait for flag set in interrupt routine
void loop (void)
{
  if (process_it)
  {
    buf [pos] = 0;  
    Serial.println (buf);
    pos = 0;
    process_it = false;
  }
    
}
