/*
 * xblastScriptEngine.c
 *
 *  Created on: Jan 14, 2015
 *      Author: cromwelldev
 */

#include "boot.h"
#include "xblastScriptEngine.h"

bool ifFunction(void * param1, void * op, void * param2);
bool elseFunction(void * ignored, void * ignored1, void * ignored2);
bool gotoFunction(void * label, void * ignored1, void * ignored2);
bool gpiFunction(void * port, void * ignored1, void * ignored2);
bool gpoFunction(void * port, void * value, void * ignored);
void waitFunction(void * ms, void * ignored1, void * ignored2);
void bootFunction(void * bank, void * ignored1, void * ignored2);
void endFunction(void * ignored, void * ignored1, void * ignored2);
void fanFunction(void * value, void * ignored1, void * ignored2);
void ledFunction(void * value, void * ignored1, void * ignored2);
void lcdPrintFunction(void * line, void * text, void * ignored);
void lcdClearLineFunction(void * line, void * ignored1, void * ignored2);
void lcdResetFunction(void * ignored, void * ignored1, void * ignored2);
void lcdBacklightFunction(void * value, void * ignored1, void * ignored2);
void lcdPowerFunction(void * value, void * ignored1, void * ignored2);

typedef struct {
    char* functionName;
    void (*functionPtr) (void *, void *, void *);
}functionCall;

void runScript(u8 * file, void * param){
    functionCall * functionCallList[] = {
            { "IF", &ifFunction},
            { "ELSE", &elseFunction},
            { "GOTO", &gotoFunction},
            { "GPI", &gpiFunction},
            { "GPO", &gpoFunction},
            { "WAIT", &waitFunction},
            { "BOOT", &bootFunction},
            { "END", &endFunction},
            { "FAN", &fanFunction},
            { "LED", &ledFunction},
            { "LCDP", &lcdPrintFunction},
            { "LCDC", &lcdClearLineFunction},
            { "LCDR", &lcdResetFunction},
            { "LCDB", &lcdBacklightFunction},
            { "LCDP", &lcdPowerFunction}
    };


    return;
}

bool ifFunction(void * param1, void * op, void * param2){
    int * tempParam1 = (int *)param1;
    int * tempParam2 = (int *)param2;
    switch(*(u8 *)op){
        case 0: //==
            return (tempParam1 == tempParam2);
            break;
        case 1: //!=
            return (tempParam1 != tempParam2);
            break;
        case 2: //>
            return (tempParam1 > tempParam2);
            break;
        case 3: //<
            return (tempParam1 < tempParam2);
            break;
        case 4: //>=
            return (tempParam1 >= tempParam2);
            break;
        case 5: //<=
            return (tempParam1 <= tempParam2);
            break;
        default:
            break;
    }
    return false;
}
bool elseFunction(void * ignored, void * ignored1, void * ignored2){
    return true;
}
bool gotoFunction(void * label, void * ignored1, void * ignored2){
    return true;
}
bool gpiFunction(void * port, void * ignored1, void * ignored2){
    LPCMod_ReadIO(NULL);
    if(*(u8 *)port == 0)
        return GenPurposeIOs.GPI0;

    return GenPurposeIOs.GPI1;
}
bool gpoFunction(void * port, void * value, void * ignored){
    LPCMod_WriteIO(*(u8 *)port, *(u8 *)value);
    return true;
}
void waitFunction(void * ms, void * ignored1, void * ignored2){
    wait_ms(*(u32 *)ms);
}
void bootFunction(void * bank, void * ignored1, void * ignored2){

}
void endFunction(void * ignored, void * ignored1, void * ignored2){

}
void fanFunction(void * value, void * ignored1, void * ignored2){
    I2CSetFanSpeed(*(u8 *)value);
}
void ledFunction(void * value, void * ignored1, void * ignored2){
    setLED((char *)value);
}
void lcdPrintFunction(void * line, void * text, void * ignored){
    switch(*(u8 *)line){
        case 0:
            WriteLCDLine0(0, (char *)text);
            break;
        case 1:
            WriteLCDLine1(0, (char *)text);
            break;
        case 2:
            WriteLCDLine2(0, (char *)text);
            break;
        default:
            WriteLCDLine3(0, (char *)text);
            break;
    }
}
void lcdClearLineFunction(void * line, void * ignored1, void * ignored2){
    WriteLCDClearLine(*(u8 *)line);
}
void lcdResetFunction(void * ignored, void * ignored1, void * ignored2){
    WriteLCDCommand(0x1);
}
void lcdBacklightFunction(void * value, void * ignored1, void * ignored2){
    setLCDBacklight(*(u8 *)value);
}
void lcdPowerFunction(void * value, void * ignored1, void * ignored2){
    toggleEN5V(*(u8 *)value);
    if(*(u8 *)value)
        assertInitLCD();
}
