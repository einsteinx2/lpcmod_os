/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "lib/cromwell/cromString.h"
#include "lib/time/timeManagement.h"
#include "boot.h"
#include "VideoInitialization.h"
#include "BootLCD.h"
#include "lpcmod_v1.h"
#include "xblast/HardwareIdentifier.h"
#include "xblast/settings/xblastSettingsDefs.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "string.h"

void BootLCDInit(void){
    xLCD.enable = 0;            //Set it unintialized for now.
    xLCD.TimingCMD = 1500;      //Arbitrary but safe.
    xLCD.TimingData = 90;

    BootLCDUpdateLinesOwnership(0, 0);
    BootLCDUpdateLinesOwnership(1, 0);
    BootLCDUpdateLinesOwnership(2, 0);
    BootLCDUpdateLinesOwnership(3, 0);
    BootLCDSwitchType();

    //Function pointers included in struct for easier access throughout the program.
    xLCD.Init = WriteLCDInit;
    xLCD.Command = WriteLCDCommand;
    xLCD.Data = WriteLCDData;
    if(isXecuter3())    //Xecuter 3 interface differently from other modchips.
        xLCD.WriteIO = X3WriteLCDIO;
    else
        xLCD.WriteIO = WriteLCDIO;
    xLCD.PrintLine[0] = WriteLCDLine0;
    xLCD.PrintLine[1] = WriteLCDLine1;
    xLCD.PrintLine[2] = WriteLCDLine2;
    xLCD.PrintLine[3] = WriteLCDLine3;
    xLCD.ClearLine = WriteLCDClearLine;
}

void BootLCDSwitchType(void){
    xLCD.LineSize = LPCmodSettings.LCDsettings.lineLength;    //Defaults to 4 lines LCDs
    xLCD.nbLines = LPCmodSettings.LCDsettings.nbLines;        //Defaults to 20 chars/line
    switch(LPCmodSettings.LCDsettings.lcdType){
        case LCDTYPE_KS0073:
            xLCD.Line1Start = 0x00;
            xLCD.Line2Start = 0x20;    //Check the datasheet if you don't believe me.
            xLCD.Line3Start = 0x40;
            xLCD.Line4Start = 0x60;
            break;
        default:                       //HD44780 by default
            xLCD.Line1Start = 0x00;    //4 lines config is good for smaller LCD too.
            xLCD.Line2Start = 0x40;
            xLCD.Line3Start = xLCD.Line1Start + LPCmodSettings.LCDsettings.lineLength;
            xLCD.Line4Start = xLCD.Line2Start + LPCmodSettings.LCDsettings.lineLength;
        }
}

void toggleEN5V(unsigned char value){
    GenPurposeIOs.EN_5V = value;
    LPCMod_WriteGenPurposeIOs();     //Write to LPC register
}

void setLCDContrast(unsigned char value){
    float fContrast=((float)value)/100.0f;
    fContrast*=127.0f;
    unsigned char newValue=(unsigned char)fContrast;
    if (newValue==63) newValue=64;
    WriteToIO(LCD_CT, newValue);
}

void setLCDBacklight(unsigned char value){
    if(isXecuter3() == false){  //Everything but Xecuter 3
        float fBackLight=((float)value)/100.0f;
        fBackLight*=127.0f;
        unsigned char newValue=(unsigned char)fBackLight;
        if (newValue==63) newValue=64;
        WriteToIO(LCD_BL, newValue);
    }
    else                                               //Xecuter 3
        WriteToIO(X3_DISP_O_LIGHT, (unsigned char)(2.55*(double)value) );
}

void assertInitLCD(void){
    if(LPCmodSettings.LCDsettings.enable5V == 1 && xLCD.enable != 1){    //Display should be ON but is not initialized.
        if(isPureXBlast())     //XBlast Mod only.
            toggleEN5V(LPCmodSettings.LCDsettings.enable5V);
        xLCD.enable = 1;
        if(isLCDContrastSupport())
            setLCDContrast(LPCmodSettings.LCDsettings.contrast);
        setLCDBacklight(LPCmodSettings.LCDsettings.backlight);
        wait_ms(10);                    //Wait a precautionary 10ms before initializing the LCD to let power stabilize.
        WriteLCDInit();
        xLCD.LineSize = LPCmodSettings.LCDsettings.lineLength;
        initialLCDPrint();
    }
    if(LPCmodSettings.LCDsettings.enable5V == 0) {
        xLCD.enable = 0;
        toggleEN5V(LPCmodSettings.LCDsettings.enable5V);
        setLCDContrast(0);
        setLCDBacklight(0);
    }
}



void WriteLCDInit(void)
{
    if(xLCD.enable != 1)
        return;

    //Xecuter 3 only
    if(isXecuter3())
    {
        //initialize GP/IO
        WriteToIO(X3_DISP_O_DAT, 0);
        WriteToIO(X3_DISP_O_CMD, 0);
        WriteToIO(X3_DISP_O_DIR_DAT, 0xFF);
        WriteToIO(X3_DISP_O_DIR_CMD, 0x07);
    }
    //It's been at least ~15ms since boot.
    //Start of init, with delay
    xLCD.WriteIO(0x33,0,4100);    //Use a single call to write twice function set 0b0011 with 4.1ms delay

    xLCD.WriteIO(0x32,0,1500);    //Again a single call to write but this time write 0b0011 in first and 0b0010 in second
                        //Second write could be shorter but meh...

    //LCD is now in 4-bit mode.
    wait_us(1);
    xLCD.Command(DISP_FUNCTION_SET | DISP_N_FLAG | DISP_RE_FLAG);    //2 lines and 5x8 dots character resolution.
    wait_us(1);
    xLCD.Command(DISP_SEGRAM_SET);            //Display OFF, Cursor OFF, Cursor blink OFF.
    wait_us(1);
    xLCD.Command(DISP_EXT_CONTROL | DISP_NW_FLAG);
    wait_us(1);
    xLCD.Command(DISP_FUNCTION_SET | DISP_N_FLAG);    //Entry mode,Increment cursor, shift right
    wait_us(1);
    xLCD.Command(DISP_CONTROL | DISP_D_FLAG);        //Display ON.
    wait_us(1);
    xLCD.Command(DISP_CLEAR);
    wait_us(1);
    xLCD.Command(DISP_ENTRY_MODE_SET | DISP_ID_FLAG);  
    wait_us(1);
    xLCD.Command(DISP_HOME);  
   
    
}

void WriteLCDCommand(unsigned char value){
    if(xLCD.enable != 1)
        return;
    xLCD.WriteIO(value, 0, xLCD.TimingCMD);        //RS=0 for commands.
}

void WriteLCDData(unsigned char value){
    if(xLCD.enable != 1)
        return;
    xLCD.WriteIO(value, isXecuter3()?X3_DISPLAY_RS:DISPLAY_RS, xLCD.TimingData);
}

void WriteLCDIO(unsigned char data, bool RS, unsigned short wait){
    unsigned char lsbNibble = 0;
    unsigned char msbNibble = 0;
    
    if(xLCD.enable != 1)
        return;
                            //data is b7,b6,b5,b4,b3,b2,b1,b0
    msbNibble = ((data >> 2) & 0x28) | RS;   //Maps     0,b6,b7,b4,b5,E,RS,0
    msbNibble |= (data >> 0) & 0x50;
    lsbNibble = ((data << 2) & 0x28) | RS;   //Maps     0,b2,b3,b0,b1,E,RS,0
    lsbNibble |= (data << 4) & 0x50;                                                    //data must be x,D7,D6,D5,D4,E,RS,x
                                                    //E signal value is added below as it's the "clock"
    //High nibble first
    //Initially place the data
    WriteToIO(LCD_DATA, msbNibble); //Place bit7,bit6,bit5,bit4,E,RS,x
    wait_us(90);    //needs to be at least 40ns
    
    msbNibble |= DISPLAY_E;
    //Raise E signal line
    WriteToIO(LCD_DATA, msbNibble); //Place bit7,bit6,bit5,bit4,E,RS,x
    wait_us(90);    //needs to be at least 230ns
    
    msbNibble ^= DISPLAY_E;
    //Drop E signal line
    WriteToIO(LCD_DATA, msbNibble); //Place bit7,bit6,bit5,bit4,E,RS,x
    wait_us(wait);
    
    //Low nibble in second
    //Initially place the data
    WriteToIO(LCD_DATA, lsbNibble); //Place bit3,bit2,bit1,bit0,E,RS,x
    wait_us(90);    //needs to be at least 40ns
    
    lsbNibble |= DISPLAY_E;
    //Raise E signal line
    WriteToIO(LCD_DATA, lsbNibble); //Place bit3,bit2,bit1,bit0,E,RS,x
    wait_us(90);    //needs to be at least 230ns
    
    lsbNibble ^= DISPLAY_E;
    //Drop E signal line
    WriteToIO(LCD_DATA, lsbNibble); //Place bit3,bit2,bit1,bit0,E,RS,x
    wait_us(wait);
}

void X3WriteLCDIO(unsigned char data, bool RS, unsigned short wait){
        unsigned char lsbNibble = 0;
        unsigned char msbNibble = 0;

        if(xLCD.enable != 1)
            return;
                                //data is b7,b6,b5,b4,b3,b2,b1,b0
        msbNibble = (data  & 0xF0);   //Maps     b6,b7,b4,b5,0,0,0,0
        lsbNibble = (data << 4);   //Maps     b2,b3,b0,b1,0,0,0,0          //data must be D7,D6,D5,D4,x,x,x,x
                                                        //E signal value is added below as it's the "clock"
        //High nibble first
        //Initially place the data
        WriteToIO(X3_DISP_O_DAT, msbNibble); //Place b6,b7,b4,b5,x,x,x,x


        WriteToIO(X3_DISP_O_CMD, RS);
        wait_us(90);    //needs to be at least 40ns
        //Raise E signal line
        WriteToIO(X3_DISP_O_CMD, RS | DISPLAY_E);
        wait_us(90);    //needs to be at least 230ns

        //Drop E signal line
        WriteToIO(X3_DISP_O_CMD, RS);
        wait_us(wait);

        //Low nibble in second
        //Initially place the data
        WriteToIO(X3_DISP_O_DAT, lsbNibble); //Place b2,b3,b0,b1,x,x,x,x
        wait_us(90);    //needs to be at least 40ns

        WriteToIO(X3_DISP_O_CMD, RS);
        wait_us(90);    //needs to be at least 40ns
        //Raise E signal line
        WriteToIO(X3_DISP_O_CMD, RS | DISPLAY_E);
        wait_us(90);    //needs to be at least 230ns

        //Drop E signal line
        WriteToIO(X3_DISP_O_CMD, RS);
        wait_us(wait);
}

void WriteLCDLine0(bool centered, char *lineText){
    int i;
    char LineBuffer[LPCmodSettings.LCDsettings.lineLength + 1];    //For the escape character at the end.

    if(xLCD.enable != 1)
        return;    
    
    if(centered){
        //Play with the string to center it on the LCD unit.
        WriteLCDCenterString(LineBuffer, lineText);
    }
    else
        WriteLCDFitString(LineBuffer, lineText);



    //Place cursor
    WriteLCDSetPos(0,0);    // Write to first line

    //Send every character of the string to LCD unit.
    for (i=0;i<xLCD.LineSize;i++) {
        xLCD.Data(LineBuffer[i]);
    }
}

void WriteLCDLine1(bool centered, char *lineText){
    int i;
    char LineBuffer[LPCmodSettings.LCDsettings.lineLength + 1];    //For the escape character at the end.

    if(xLCD.enable != 1 || xLCD.nbLines <= 1)
        return;

    if(centered){
        //Play with the string to center it on the LCD unit.
        WriteLCDCenterString(LineBuffer, lineText);
    }
    else
        WriteLCDFitString(LineBuffer, lineText);


    //Place cursor
    WriteLCDSetPos(0,1);    // Write to second line

    //Send every character of the string to LCD unit.
    for (i=0;i<xLCD.LineSize;i++) {
        xLCD.Data(LineBuffer[i]);
    }
}

void WriteLCDLine2(bool centered, char *lineText){
    int i;
    char LineBuffer[LPCmodSettings.LCDsettings.lineLength + 1];    //For the escape character at the end.

    if(xLCD.enable != 1 || xLCD.nbLines <= 2)
        return;

    if(centered){
        //Play with the string to center it on the LCD unit.
        WriteLCDCenterString(LineBuffer, lineText);
    }
    else
        WriteLCDFitString(LineBuffer, lineText);

    //Place cursor
    WriteLCDSetPos(0,2);    // Write to third line

    //Send every character of the string to LCD unit.
    for (i=0;i<xLCD.LineSize;i++) {
        xLCD.Data(LineBuffer[i]);
    }
}

void WriteLCDLine3(bool centered, char *lineText){
    int i;
    char LineBuffer[LPCmodSettings.LCDsettings.lineLength + 1];    //For the escape character at the end.

    if(xLCD.enable != 1 || xLCD.nbLines <= 2)
        return;

    if(centered){
        //Play with the string to center it on the LCD unit.
        WriteLCDCenterString(LineBuffer, lineText);
    }
    else
        WriteLCDFitString(LineBuffer, lineText);


    //Place cursor
    WriteLCDSetPos(0,3);    // Write to fourth line

    //Send every character of the string to LCD unit.
    for (i=0;i<xLCD.LineSize;i++) {
        xLCD.Data(LineBuffer[i]);
    }
}


void WriteLCDCenterString(char * StringOut, char * stringIn){
    int i;
    memset(StringOut,0x0,xLCD.LineSize);    //Let's get clean a little.

    if(xLCD.enable != 1)
        return;

    //Skip first character.
    if (stringIn[0]=='\2') {
        strncpy(StringOut,&stringIn[1],xLCD.LineSize - 1);        //We skipped the first character.
    } else {
        strncpy(StringOut,stringIn,xLCD.LineSize);    //Line length is variable
    }
    StringOut[xLCD.LineSize] = 0;            //Escape character at the end that's for sure
    i = strlen(StringOut);
    //String length is shorter than what can be displayed on a single line of the LCD unit.
    if (i < xLCD.LineSize) {
        char szTemp1[xLCD.LineSize];
        unsigned char rest = (xLCD.LineSize-i) / 2;

        //Print "space"(0x20 in ascii) in the whole array.
        memset(szTemp1,0x20,xLCD.LineSize);
        memcpy(&szTemp1[rest],StringOut,i);    //Place actual text in the middle of the array.
        memcpy(StringOut,szTemp1,xLCD.LineSize);
        //LineBuffer now contains our text, centered on a single LCD unit's line.
    }
}

void WriteLCDFitString(char * StringOut, char * stringIn){
    int i;
    memset(StringOut,0x0,xLCD.LineSize);    //Let's get clean a little.

    if(xLCD.enable != 1)
        return;

    //Skip first character.
    if (stringIn[0]=='\2') {
        strncpy(StringOut,&stringIn[1],xLCD.LineSize - 1);        //We skipped the first character.
    } else {
        strncpy(StringOut,stringIn,xLCD.LineSize);    //Line length is variable
    }
    StringOut[xLCD.LineSize] = 0;            //Escape character at the end that's for sure
    i = strlen(StringOut);
    //String length is shorter than what can be displayed on a single line of the LCD unit.
    if (i < xLCD.LineSize) {
        char szTemp1[xLCD.LineSize];

        //Print "space"(0x20 in ascii) in the whole array.
        memset(szTemp1,0x20,xLCD.LineSize);
        memcpy(szTemp1,StringOut,i);    //Place actual text justified to the left.
        memcpy(StringOut,szTemp1,xLCD.LineSize);
        //LineBuffer now contains our text, centered on a single LCD unit's line.
    }
}

void WriteLCDSetPos(unsigned char pos, unsigned char line) {
    unsigned char cursorPtr = pos % xLCD.LineSize;

  if(xLCD.enable != 1)
        return;    
    
  if (line == 0) {
    cursorPtr += xLCD.Line1Start;
  }
  if (line == 1) {
    cursorPtr += xLCD.Line2Start;
  }
  if (line == 2) {
    cursorPtr += xLCD.Line3Start;
  }
  if (line == 3) {
    cursorPtr += xLCD.Line4Start;
  }

  xLCD.Command(DISP_DDRAM_SET | cursorPtr);
    
}

void WriteLCDClearLine(unsigned char line) {
    char empty[xLCD.LineSize];
    
    if(xLCD.enable != 1)
        return;
        
    memset(empty,' ',xLCD.LineSize);

    //Call the proper function for the desired line.
    xLCD.PrintLine[line](JUSTIFYLEFT, empty);
}

void initialLCDPrint(void){
    if(LPCmodSettings.LCDsettings.customTextBoot == 1){
        xLCD.PrintLine[0](JUSTIFYLEFT, LPCmodSettings.LCDsettings.customString0);
        xLCD.PrintLine[1](JUSTIFYLEFT, LPCmodSettings.LCDsettings.customString1);
        xLCD.PrintLine[2](JUSTIFYLEFT, LPCmodSettings.LCDsettings.customString2);
        xLCD.PrintLine[3](JUSTIFYLEFT, LPCmodSettings.LCDsettings.customString3);
    }
    else{
        xLCD.Command(DISP_CLEAR);
        xLCD.PrintLine[0](CENTERSTRING, "XBlast mod V1");
    }
}

void BootLCDUpdateLinesOwnership(unsigned char line, unsigned char fromScript){
    char customStringsFirstCharacterArray[4] = {LPCmodSettings.LCDsettings.customString0[0], LPCmodSettings.LCDsettings.customString1[0], LPCmodSettings.LCDsettings.customString2[0], LPCmodSettings.LCDsettings.customString3[0]};

    if (fromScript == SCRIPT_OWNER){
        xLCD.LineOwner[line] = SCRIPT_OWNER;
        return;
    }

    if(xLCD.LineOwner[line] != SCRIPT_OWNER){
        if(LPCmodSettings.LCDsettings.customTextBoot && customStringsFirstCharacterArray[line] != '\0')
            xLCD.LineOwner[line] = CUSTOM_OWNER;
        else
            xLCD.LineOwner[line] = SYSTEM_OWNER;
    }
}
