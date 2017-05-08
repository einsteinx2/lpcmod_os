/*
 * functionsAccessor.c
 *
 *  Created on: May 6, 2017
 *      Author: bennyboy
 */

#include "../../xblast/scriptEngine/functionsAccessor.h"
#include <string.h>
#include <stdio.h>

bool gpiFunction(unsigned char port)
{
    return true;
}

bool gpoFunction(unsigned char port, unsigned char value)
{
    return true;
}

bool waitFunction(int ms)
{
    return true;
}

bool bootFunction(FlashBank bank)
{
    return true;
}

bool fanFunction(unsigned char value)
{
    return true;
}

bool ledFunction(char * value)
{
    printf("LED %s\n", value);
    return true;
}

bool lcdPrintFunction(unsigned char line, char * text, unsigned char stringLength)
{
    return true;
}

bool lcdClearLineFunction(unsigned char line)
{
    return true;
}
bool lcdResetFunction(void)
{
    return true;
}

bool lcdBacklightFunction(unsigned char value)
{
    return true;
}

bool lcdPowerFunction(unsigned char value)
{
    return true;
}

unsigned char SPIRead(void)
{
    unsigned char i, result = 0;

    return result;
}

bool SPIWrite(unsigned char data)
{
    return true;
}

unsigned char XPADRead(int xpadVariable)
{
    unsigned char pad = 1;
    return pad;
}

bool emergencyEscape()
{
    return false;
}
