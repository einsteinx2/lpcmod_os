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

	//Start of init, with delay
	//xLCD->WriteIO(xLCD,(u32)0x1001F);

	//Switch to 4-bit mode
	WriteLCDIO_strobe(xLCD,DISPLAY_FUNCTION_SET | DISPLAY_DL_FLAG,0,4000);	//Arbitrary delay value.
	WriteLCDIO_strobe(xLCD,DISPLAY_FUNCTION_SET | DISPLAY_DL_FLAG,0,1500);	//Could probably be shorter
	WriteLCDIO_strobe(xLCD,DISPLAY_FUNCTION_SET | DISPLAY_DL_FLAG,0,1500);	//but meh..

	//init display driver
	WriteLCDIO_strobe(xLCD,DISPLAY_FUNCTION_SET,0,1500);

	xLCD->Command(xLCD,DISP_FUNCTION_SET | DISP_N_FLAG | DISP_RE_FLAG);
	xLCD->Command(xLCD,DISP_SEGRAM_SET);
	xLCD->Command(xLCD,DISP_EXT_CONTROL | DISP_NW_FLAG);


	xLCD->Command(xLCD,DISP_FUNCTION_SET | DISP_N_FLAG);	// Set interface length and 1 line
	xLCD->Command(xLCD,DISP_CONTROL | DISP_D_FLAG);
	xLCD->Command(xLCD,DISP_CLEAR);
	xLCD->Command(xLCD,DISP_ENTRY_MODE_SET | DISP_ID_FLAG);

	//Place cursor at the begining (character 0 of first line).
	xLCD->Command(xLCD, 0x80);

	return;
}

void WriteLCDCommand(struct Disp_controller *xLCD, u8 value){
	u8 temp = value & 0xf0;

	//Send higher nibble
	temp = temp >> 4;
	WriteLCDIO_strobe(xLCD, temp, 0, xLCD->TimingCMD);

	//Send lower nibble
	temp = value&0x0f;
	WriteLCDIO_strobe(xLCD,temp,0,xLCD->TimingCMD);

	return;
}

void WriteLCDData(struct Disp_controller *xLCD, u8 value){
	u8 temp = value & 0xf0;
	// Send higher nibble
	temp = temp >>4;
	WriteLCDIO_strobe(xLCD,temp, DISPLAY_RS, xLCD->TimingData);
	// We send the Lower Bits
	temp = value & 0x0f;
	WriteLCDIO_strobe(xLCD,temp, DISPLAY_RS, xLCD->TimingData);
}

void WriteLCDIO(struct Disp_controller *xLCD, u8 data, bool RS, u16 wait){
	u8 lsbNibble = 0;
	u8 msbNibble = 0;
	
	lsbNibble = ((value & 0x0f) << 3) | (RS << 1);	
	msbNibble = ((value & 0xf0) >> 1) | (RS << 1);
	
	//High nibble first
	//Initially place the data
	WriteToIO(LCD_DATA, msbNibble); //Place bit7,bit6,bit5,bit4,E,RS,x	
	waitus(delay);	
	
	msbNibble |= DISPLAY_E;
	//Raise E signal line
	WriteToIO(LCD_DATA, msbNibble); //Place bit7,bit6,bit5,bit4,E,RS,x	
	waitus(delay);
	
	msbNibble ^= DISPLAY_E;
	//Drop E signal line
	WriteToIO(LCD_DATA, msbNibble); //Place bit7,bit6,bit5,bit4,E,RS,x	
	waitus(delay);
	
	//Low nibble in second
	//Initially place the data
	WriteToIO(LCD_DATA, lsbNibble); //Place bit3,bit2,bit1,bit0,E,RS,x
	waitus(delay);	
	
	lsbNibble |= DISPLAY_E;
	//Raise E signal line
	WriteToIO(LCD_DATA, lsbNibble); //Place bit3,bit2,bit1,bit0,E,RS,x
	waitus(delay);
	
	lsbNibble ^= DISPLAY_E;
	//Drop E signal line
	WriteToIO(LCD_DATA, lsbNibble); //Place bit3,bit2,bit1,bit0,E,RS,x
	waitus(delay);
	
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
