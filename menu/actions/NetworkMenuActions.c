/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "NetworkMenuActions.h"
#include "boot.h"

void toggleUseDHCP(void * itemStr) {
    (LPCmodSettings.OSsettings.useDHCP) = (LPCmodSettings.OSsettings.useDHCP)? 0 : 1;
    sprintf(itemStr,"%s",LPCmodSettings.OSsettings.useDHCP? "Yes" : "No");
}

void editStaticIPfield(void * ip) {
    u8 * addr = (u8 *) ip;
    u8 cursorPos = 0, byteOffset = 0, tempAddr[4], countDots = 0, lastDotPos;
    char tempStringIP[16];      //+1 for terminating character

    sprintf(tempStringIP, "%u.%u.%u.%u", addr[0], addr[1], addr[2], addr[3]);
    OnScreenKeyboard(tempStringIP, 15, 3, IP_KEYPAD);
    while(tempStringIP[cursorPos] != '\0') {

        if(cursorPos == 0 && tempStringIP[cursorPos] != '.')      //First byte doesn't have a '.' in before.
            tempAddr[byteOffset++] = myAtoi(tempStringIP);
        else if(tempStringIP[cursorPos] == '.' && tempStringIP[cursorPos + 1] != '.' && tempStringIP[cursorPos + 1] != '\0') {       //Others do.
            tempAddr[byteOffset++] = myAtoi(&tempStringIP[cursorPos + 1]);
            countDots += 1;
            lastDotPos = cursorPos;
        }
        cursorPos += 1; //First character cannot be a '.'
    }
    if(countDots == 3 && tempStringIP[lastDotPos + 1] != '\0'){
        addr[0] = tempAddr[0];
        addr[1] = tempAddr[1];
        addr[2] = tempAddr[2];
        addr[3] = tempAddr[3];
    }
}

u8 myAtoi(char *str)
{
    u8 i, res = 0;

    for (i = 0; str[i] != '\0' && str[i] != '.'; ++i)   //Will stop converting if it hit a '.' or the end of the string.
        res = res*10 + str[i] - '0';

    return res;
}
