#include "boot.h"
#include "VideoInitialization.h"
#include "BootLCD.h"
#include "lpcmod_v1.h"
#include "BootLPCMod.h"

void BootLCDInit(void){
	xLCD->enable = 0;	//Set it unintialized for now.
	xLCD->LineSize = HDD4780_DEFAULT_LINELGTH;	//Default for common 4 lines LCDs
	xLCD->TimingCMD = 1000;						//Arbitrary but safe.
	xLCD->TimingData = 50;
	xLCD->Line1Start = 0x00;
	xLCD->Line2Start = 0x40;	//Check the datasheet if you don't believe me.
	xLCD->Line3Start = 0x14;
	xLCD->Line4Start = 0x54;


	//Function pointers included in struct for easier access throughout the program.
	xLCD->Init = WriteLCDInit;
	xLCD->Command = WriteLCDCommand;
	xLCD->Data = WriteLCDData;
	xLCD->WriteIO = WriteLCDIO;
	xLCD->Poll = WriteLCDPoll;
	xLCD->PrintLine1 = WriteLCDLine1;
	xLCD->PrintLine2 = WriteLCDLine2;
	xLCD->PrintLine3 = WriteLCDLine3;
	xLCD->PrintLine4 = WriteLCDLine4;
}

void toggleEN5V(u8 value){
	WriteToIO(ENABLE_5V, value);
}

void setLCDContrast(u8 value){
	WriteToIO(LCD_CT, value);
}

void setLCDBacklight(u8 value){
	WriteToIO(LCD_BL, value);
}

void assertInitLCD(void){
	if(LPCmodSettings.LCDsettings.enable5V && xLCD->enable == 0){	//Display should be ON but is not initialized.
		toggleEN5V(LPCmodSettings.LCDsettings.enable5V);
		setLCDContrast(LPCmodSettings.LCDsettings.contrast);
		setLCDBacklight(LPCmodSettings.LCDsettings.backlight);
		xLCD->Init(xLCD);
		xLCD->LineSize = LPCmodSettings.LCDsettings.lineLength;
		xLCD->PrintLine1(xLCD, "LPCMod V1");				//Remove or change after proven working.
		xLCD->PrintLine2(xLCD, "Yay LCD!");
		xLCD->PrintLine3(xLCD, "OS dev is going well");
		xLCD->PrintLine4(xLCD, "-bennydiamond");
	}
	xLCD->enable = LPCmodSettings.LCDsettings.enable5V;		//Whatever happens, this statement will be valid.
}



void WriteLCDInit(struct Disp_controller *xLCD){

	//It's been at least 15ms since boot.
	//Start of init, with delay
	xLCD->WriteIO(xLCD,0x33,0,4100);		//Use a single call to write twice function set 0b0011 with 4.1ms delay
	xLCD->WriteIO(xLCD,0x32,0,1500);		//Again a single call to write but this time write 0b0011 in first and 0b0010 in second
											//Second write could be shorter but meh...

	//LCD is now in 4-bit mode.

	xLCD->Command(xLCD,DISP_FUNCTION_SET | DISP_N_FLAG | DISP_RE_FLAG);	//2 lines and 5x10 dots character resolution.
	xLCD->Command(xLCD,DISP_SEGRAM_SET);								//Set CGRAM address to 0x0
	xLCD->Command(xLCD,DISP_CONTROL | DISP_NW_FLAG);				//Display OFF, Cursor OFF, Cursor blink ON.


	xLCD->Command(xLCD,DISP_FUNCTION_SET | DISP_N_FLAG);	// Set interface length and 1 line
	xLCD->Command(xLCD,DISP_CONTROL | DISP_D_FLAG);			//Display ON.
	xLCD->Command(xLCD,DISP_CLEAR);
	xLCD->Command(xLCD,DISP_ENTRY_MODE_SET | DISP_ID_FLAG);

	//Place cursor at the begining (character 0 of first line).
	xLCD->Command(xLCD, 0x80);

	return;
}

void WriteLCDCommand(struct Disp_controller *xLCD, u8 value){
	xLCD->WriteIO(xLCD, value, 0, xLCD->TimingCMD);		//RS=0 for commands.
	return;
}

void WriteLCDData(struct Disp_controller *xLCD, u8 value){
	xLCD->WriteIO(xLCD,value, DISPLAY_RS, xLCD->TimingData);
}

void WriteLCDIO(struct Disp_controller *xLCD, u8 data, bool RS, u16 wait){
	u8 lsbNibble = 0;
	u8 msbNibble = 0;
													//data is b7,b6,b5,b4,b3,b2,b1,b0
    lsbNibble = ((data & 0x0f) << 3) | (RS << 1);   //Maps         0,b3,b2,b1,b0,E,RS,0
    msbNibble = ((data & 0xf0) >> 1) | (RS << 1);   //Maps         0,b7,b6,b5,b4,E,RS,0
	                                                //data must be x,D7,D6,D5,D4,E,RS,x
    												//E signal value is added below as it's the "clock"
	//High nibble first
	//Initially place the data
	WriteToIO(LCD_DATA, msbNibble); //Place bit7,bit6,bit5,bit4,E,RS,x	
	wait_us(1);	//needs to be at least 40ns
	
	msbNibble |= DISPLAY_E;
	//Raise E signal line
	WriteToIO(LCD_DATA, msbNibble); //Place bit7,bit6,bit5,bit4,E,RS,x	
	wait_us(1);	//needs to be at least 230ns
	
	msbNibble ^= DISPLAY_E;
	//Drop E signal line
	WriteToIO(LCD_DATA, msbNibble); //Place bit7,bit6,bit5,bit4,E,RS,x	
	wait_us(wait);
	
	//Low nibble in second
	//Initially place the data
	WriteToIO(LCD_DATA, lsbNibble); //Place bit3,bit2,bit1,bit0,E,RS,x
	wait_us(1);	//needs to be at least 40ns
	
	lsbNibble |= DISPLAY_E;
	//Raise E signal line
	WriteToIO(LCD_DATA, lsbNibble); //Place bit3,bit2,bit1,bit0,E,RS,x
	wait_us(1);	//needs to be at least 230ns
	
	lsbNibble ^= DISPLAY_E;
	//Drop E signal line
	WriteToIO(LCD_DATA, lsbNibble); //Place bit3,bit2,bit1,bit0,E,RS,x
	wait_us(wait);
	
	return;
}

void WriteLCDPoll(struct Disp_controller *xLCD){
	return;
}

void WriteLCDLine1(struct Disp_controller *xLCD, char *lineText){
	int i;
	char LineBuffer[LPCmodSettings.LCDsettings.lineLength + 1];	//For the escape character at the end.

	//Play with the string to center it on the LCD unit.
	WriteLCDTrimString(LineBuffer, lineText);


	//Place cursor
	xLCD->Command(xLCD,xLCD->Line1Start|0x80);	// Write to first line

	//Send every character of the string to LCD unit.
	for (i=0;i<xLCD->LineSize;i++) {
		xLCD->Data(xLCD,LineBuffer[i]);
	}
}

void WriteLCDLine2(struct Disp_controller *xLCD, char *lineText){
	int i;
	char LineBuffer[LPCmodSettings.LCDsettings.lineLength + 1];	//For the escape character at the end.

	//Play with the string to center it on the LCD unit.
	WriteLCDTrimString(LineBuffer, lineText);


	//Place cursor
	xLCD->Command(xLCD,xLCD->Line2Start|0x80);	// Write to first line

	//Send every character of the string to LCD unit.
	for (i=0;i<xLCD->LineSize;i++) {
		xLCD->Data(xLCD,LineBuffer[i]);
	}
}

void WriteLCDLine3(struct Disp_controller *xLCD, char *lineText){
	int i;
	char LineBuffer[LPCmodSettings.LCDsettings.lineLength + 1];	//For the escape character at the end.

	//Play with the string to center it on the LCD unit.
	WriteLCDTrimString(LineBuffer, lineText);


	//Place cursor
	xLCD->Command(xLCD,xLCD->Line3Start|0x80);	// Write to first line

	//Send every character of the string to LCD unit.
	for (i=0;i<xLCD->LineSize;i++) {
		xLCD->Data(xLCD,LineBuffer[i]);
	}
}

void WriteLCDLine4(struct Disp_controller *xLCD, char *lineText){
	int i;
	char LineBuffer[LPCmodSettings.LCDsettings.lineLength + 1];	//For the escape character at the end.

	//Play with the string to center it on the LCD unit.
	WriteLCDTrimString(LineBuffer, lineText);


	//Place cursor
	xLCD->Command(xLCD,xLCD->Line4Start|0x80);	// Write to first line

	//Send every character of the string to LCD unit.
	for (i=0;i<xLCD->LineSize;i++) {
		xLCD->Data(xLCD,LineBuffer[i]);
	}
}

u8 WriteLCDNibbleGen(u8 value){
	u8 temp,temp2;
	temp2 = value;
	value = value&0x0f;
	temp = 0;
	temp |= (value& (1<<0) )?(1<<4):0;
	temp |= (value& (1<<1) )?(1<<3):0;
	temp |= (value& (1<<2) )?(1<<6):0;
	temp |= (value& (1<<3) )?(1<<5):0;

	temp |= (temp2>>8)&0xff;

	return temp;	//All good to go.
}


void WriteLCDIO_strobe(struct Disp_controller *xLCD, u8 data, u8 flag, u32 waitTime){
	//xLCD->WriteIO(xLCD, ((waitTime << 16) | (flag << 8) | data));

	flag |= DISPLAY_E;
	//xLCD->WriteIO(xLCD, ((waitTime << 16) | (flag << 8) | data));

	flag ^= DISPLAY_E;		//Drop E signal line low
	//xLCD->WriteIO(xLCD, ((waitTime << 16) | (flag << 8) | data));
}

void WriteLCDTrimString(char * StringOut, char * stringIn){
	int i;
	memset(StringOut,0x0,LPCmodSettings.LCDsettings.lineLength);	//Let's get clean a little.

	//Skip first character.
	if (stringIn[0]=='\2') {
		strncpy(StringOut,&stringIn[1],LPCmodSettings.LCDsettings.lineLength - 1);		//We skipped the first character.
	} else {
		strncpy(StringOut,stringIn,LPCmodSettings.LCDsettings.lineLength);	//Line length is 20 characters
	}
	StringOut[LPCmodSettings.LCDsettings.lineLength] = 0;			//Escape character at the end(21st character) that's for sure
	i = strlen(StringOut);
	//String length is shorter than what can be displayed on a single line of the LCD unit.
	if (i < xLCD->LineSize) {
		char szTemp1[LPCmodSettings.LCDsettings.lineLength];
		u8 rest = (xLCD->LineSize-i) / 2;

		//Print "space"(0x20 in ascii) in the whole array.
		memset(szTemp1,0x20,LPCmodSettings.LCDsettings.lineLength);
		memcpy(&szTemp1[rest],StringOut,i);	//Place actual text in the middle of the array.
		memcpy(StringOut,szTemp1,xLCD->LineSize);
		//LineBuffer now contains our text, centered on a single LCD unit's line.
	}
}
