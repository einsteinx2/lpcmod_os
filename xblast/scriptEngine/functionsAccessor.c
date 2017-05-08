/*
 * functionsAccessor.c
 *
 *  Created on: May 6, 2017
 *      Author: bennyboy
 */

#include "boot.h"
#include "functionsAccessor.h"
#include "lib/LPCMod/BootLCD.h"
#include "lib/cromwell/cromSystem.h"
#include "i2c.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "lib/time/timeManagement.h"
#include "xblast/PowerManagement.h"
#include "string.h"

bool gpiFunction(unsigned char port){
    //printf("\n****GPI function called, return 1");
    LPCMod_ReadIO(NULL);
    if(port)
        return GenPurposeIOs.GPI1;
    return GenPurposeIOs.GPI0;
}

bool gpoFunction(unsigned char port, unsigned char value){
    //printf("\n****Set GPO port 0x%X with value 0x%X", port, value);
    LPCMod_WriteIO(port, value);
    return true;
}

bool waitFunction(int ms){

    unsigned int startTime = getMS();

    while(cromwellLoop())
    {
        if(getMS() >= (startTime + ms))
        {
            break;
        }
    }
    //printk("\n     wait function called : %ums",ms);
    return true;
}

bool bootFunction(FlashBank bank){
    //printf("\n****Boot bank: %u", bank);
    //printk("\n     boot function called : %u",bank);
    if(bank == FlashBank_512Bank || bank == FlashBank_256Bank || bank == FlashBank_OSBank){
        BootModBios(bank);
    }
    else if(bank == FlashBank_SplitTSOP0Bank || bank == FlashBank_SplitTSOP1Bank || bank == FlashBank_FullTSOPBank){
        BootOriginalBios(bank);
    }
    else
        return false;
    return true;
}
bool fanFunction(unsigned char value){
    //printf("\n****Fan speed: %u", value);
    //printk("\n     fan function called : %u\%",value);
    if(value >=10 && value <= 100)
        I2CSetFanSpeed(value);
    return true;
}
bool ledFunction(char * value){
    //printk("\n     LED function called : %s", value);
    //printf("\n****LED pattern: %s", value);
    setLED(value);
    return true;
}
bool lcdPrintFunction(unsigned char line, char * text, unsigned char stringLength){
    //printf("\n****LCD Print at line %u : %s", line, text);
    //printk("\n     lcdPrint function called : %s",text);
    char tempString[xLCD.LineSize + 1];
    unsigned char inputStringLength;
    if(line > (xLCD.nbLines - 1))
        return false;

    if(stringLength < xLCD.LineSize){
        inputStringLength = strlen(text);
        if(inputStringLength < stringLength)
            stringLength = inputStringLength;
    }
    else{
        stringLength = xLCD.LineSize;
    }

    strncpy(tempString, text, stringLength);
    tempString[stringLength] = '\0';

    BootLCDUpdateLinesOwnership(line, SCRIPT_OWNER);
    xLCD.PrintLine[line](0, tempString);      //0 for justify text on left

    return true;
}
bool lcdClearLineFunction(unsigned char line){
    //printk("\n     lcdClearLine function called : %u",line);
    //printf("\n****LCD clear line %u", line);
    if(line > (xLCD.nbLines - 1))
        return false;

    WriteLCDClearLine(line);
    return true;
}
bool lcdResetFunction(void){
    //printf("\n****LCD reset screen");
    //printk("\n     lcdReset function called");
    WriteLCDCommand(0x01);      //CLEAR command
    return true;
}
bool lcdBacklightFunction(unsigned char value){
    //printk("\n     lcdBacklight function called : %u\%",value);
    //printf("\n****LCD backlight value: %u", value);
    setLCDBacklight(value);
    return true;
}
bool lcdPowerFunction(unsigned char value){
    //printk("\n     lcdPower function called : %u",value);
    //printf("\n****LCD power %s", value ? "ON": "OFF");
    LPCmodSettings.LCDsettings.enable5V = (value? 1 : 0);
    assertInitLCD();
    return true;
}

unsigned char SPIRead(void){
    unsigned char i, result = 0;
    for(i = 0; i < 8; i++){
        result = result << 1;
        LPCMod_FastWriteIO(0x2, 0);     //Reset CLK to 0
        //wait_us(1);
        LPCMod_FastWriteIO(0x2, 0x2);
        LPCMod_ReadIO(NULL);
        result |= GenPurposeIOs.GPI1 << i;
        //wait_us(1);    //This will need to be verified.
    }
    return result;
}

bool SPIWrite(unsigned char data){
    char i;
    for(i = 7; i >= 0; i--){
        //LPCMod_WriteIO(0x2, 0);     //Reset CLK to 0
        LPCMod_FastWriteIO(0x3, (data >> i)&0x01);
        //wait_us(1);
        LPCMod_FastWriteIO(0x2, 0x2);
        //wait_us(1);
    }
    LPCMod_FastWriteIO(0x3, 0);
    return true;
}

unsigned char XPADRead(int xpadVariable){
    //printf("\n****Controller Pad read");
    xpadVariable--;
    if(xpadVariable >= TRIGGER_XPAD_KEY_A && xpadVariable <= TRIGGER_XPAD_KEY_WHITE)
    {
        if(risefall_xpad_BUTTON(xpadVariable) == 1)
        {
            return xpadVariable + 1;
        }
    }
    return 0;
}

bool emergencyEscape(void)
{
    if(risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_RIGHT) &&
           risefall_xpad_BUTTON(TRIGGER_XPAD_TRIGGER_LEFT) &&
           risefall_xpad_STATE(XPAD_STATE_START)&&
           XPAD_current[0].keys[5])
    {
        return true;
    }

    return false;
}
