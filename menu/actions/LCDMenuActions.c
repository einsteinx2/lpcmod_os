/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "boot.h"
#include "LCDMenuActions.h"
#include "lpcmod_v1.h"
#include "lib/LPCMod/BootLCD.h"
#include "TextMenu.h"
#include "string.h"
#include "stdio.h"
#include "xblast/settings/xblastSettingsDefs.h"
#include "menu/misc/OnScreenKeyboard.h"

void LCDToggleEN5V(void * itemStr){
    LPCmodSettings.LCDsettings.enable5V = LPCmodSettings.LCDsettings.enable5V? 0 : 1;
    strcpy(itemStr, LPCmodSettings.LCDsettings.enable5V? "Yes" : "No");
    assertInitLCD();
}

void LCDChangeLCDType(void * itemStr){
    BootLCDSwitchType();
    LPCmodSettings.LCDsettings.lcdType = LPCmodSettings.LCDsettings.lcdType? LCDTYPE_HD44780 : LCDTYPE_KS0073;
    strcpy(itemStr, LPCmodSettings.LCDsettings.lcdType? "KS0073" : "HD44780");
}

void LCDIncrementBacklight(void * itemStr){
    if(LPCmodSettings.LCDsettings.backlight < 100)
        LPCmodSettings.LCDsettings.backlight += 1;
    sprintf(itemStr, "%d%%", LPCmodSettings.LCDsettings.backlight);
    if(LPCmodSettings.LCDsettings.enable5V)
        setLCDBacklight(LPCmodSettings.LCDsettings.backlight);
}

void LCDDecrementBacklight(void * itemStr){
    if(LPCmodSettings.LCDsettings.backlight > 0)
        LPCmodSettings.LCDsettings.backlight -= 1;
    sprintf(itemStr, "%d%%", LPCmodSettings.LCDsettings.backlight);
    if(LPCmodSettings.LCDsettings.enable5V)
        setLCDBacklight(LPCmodSettings.LCDsettings.backlight);
}

void LCDIncrementContrast(void * itemStr){
    if(LPCmodSettings.LCDsettings.contrast < 100)
        LPCmodSettings.LCDsettings.contrast += 1;
    sprintf(itemStr, "%d%%", LPCmodSettings.LCDsettings.contrast);
    if(LPCmodSettings.LCDsettings.enable5V)
        setLCDContrast(LPCmodSettings.LCDsettings.contrast);
}

void LCDDecrementContrast(void * itemStr){
    if(LPCmodSettings.LCDsettings.contrast > 0)
        LPCmodSettings.LCDsettings.contrast -= 1;
    sprintf(itemStr, "%d%%", LPCmodSettings.LCDsettings.contrast);
    if(LPCmodSettings.LCDsettings.enable5V)
        setLCDContrast(LPCmodSettings.LCDsettings.contrast);
}

void LCDToggleDisplayBootMsg(void * itemStr){
    LPCmodSettings.LCDsettings.displayMsgBoot = LPCmodSettings.LCDsettings.displayMsgBoot? 0 : 1;
    strcpy(itemStr, LPCmodSettings.LCDsettings.displayMsgBoot? "Yes" : "No");
}

void LCDToggledisplayBIOSNameBoot(void * itemStr){
    LPCmodSettings.LCDsettings.displayBIOSNameBoot = LPCmodSettings.LCDsettings.displayBIOSNameBoot? 0 : 1;
    strcpy(itemStr, LPCmodSettings.LCDsettings.displayBIOSNameBoot? "Yes" : "No");
}

void LCDToggledisplayCustomTextBoot(void * itemStr){
    LPCmodSettings.LCDsettings.customTextBoot = LPCmodSettings.LCDsettings.customTextBoot? 0 : 1;
    strcpy(itemStr, LPCmodSettings.LCDsettings.customTextBoot? "Yes" : "No");
    initialLCDPrint();
}

void editCustomString0(void *whatever){
    unsigned char line = 3;
    if(LPCmodSettings.LCDsettings.customTextBoot){
        line = 0;
    }
    OnScreenKeyboard(LPCmodSettings.LCDsettings.customString0, LPCmodSettings.LCDsettings.lineLength, line, FULL_KEYBOARD);
    BootLCDUpdateLinesOwnership(0, 0);
    if(LPCmodSettings.LCDsettings.customTextBoot && xLCD.LineOwner[0] != SCRIPT_OWNER){
        xLCD.PrintLine[0](JUSTIFYLEFT,LPCmodSettings.LCDsettings.customString0);
    }
//    else{
//        xLCD.ClearLine(line);
//    }
}

void editCustomString1(void *whatever){
    unsigned char line = 3;
    if(LPCmodSettings.LCDsettings.customTextBoot){
        line = 1;
    }
    OnScreenKeyboard(LPCmodSettings.LCDsettings.customString1, LPCmodSettings.LCDsettings.lineLength, line, FULL_KEYBOARD);
    BootLCDUpdateLinesOwnership(1, 0);
    if(LPCmodSettings.LCDsettings.customTextBoot && xLCD.LineOwner[1] != SCRIPT_OWNER){
        xLCD.PrintLine[1](JUSTIFYLEFT,LPCmodSettings.LCDsettings.customString1);
    }
//    else{
//        xLCD.ClearLine(line);
//    }
}

void editCustomString2(void *whatever){
    unsigned char line = 3;
    if(LPCmodSettings.LCDsettings.customTextBoot){
        line = 2;
    }
    OnScreenKeyboard(LPCmodSettings.LCDsettings.customString2, LPCmodSettings.LCDsettings.lineLength, line, FULL_KEYBOARD);
    BootLCDUpdateLinesOwnership(2, 0);
    if(LPCmodSettings.LCDsettings.customTextBoot && xLCD.LineOwner[2] != SCRIPT_OWNER){
        xLCD.PrintLine[2](JUSTIFYLEFT,LPCmodSettings.LCDsettings.customString2);
    }
//    else{
//        xLCD.ClearLine(line);
//    }
}

void editCustomString3(void *whatever){
    unsigned char line = 3;
    OnScreenKeyboard(LPCmodSettings.LCDsettings.customString3, LPCmodSettings.LCDsettings.lineLength, line, FULL_KEYBOARD);
    BootLCDUpdateLinesOwnership(3, 0);
    if(LPCmodSettings.LCDsettings.customTextBoot && xLCD.LineOwner[3] != SCRIPT_OWNER){
        xLCD.PrintLine[3](JUSTIFYLEFT,LPCmodSettings.LCDsettings.customString3);
    }
//    else{
//        xLCD.ClearLine(line);
//    }
}

void LCDDecreaseNbLines(void * itemStr){
    if(LPCmodSettings.LCDsettings.nbLines > 1)
        LPCmodSettings.LCDsettings.nbLines = LPCmodSettings.LCDsettings.nbLines >> 1;
    else
        LPCmodSettings.LCDsettings.nbLines = 1;

    BootLCDSwitchType();
    initialLCDPrint();
    sprintf(itemStr,"%u", LPCmodSettings.LCDsettings.nbLines);
}

void LCDIncreaseNbLines(void * itemStr){
    if(LPCmodSettings.LCDsettings.nbLines == 0)
        LPCmodSettings.LCDsettings.nbLines = 1;
    else if(LPCmodSettings.LCDsettings.nbLines < 4)
        LPCmodSettings.LCDsettings.nbLines = LPCmodSettings.LCDsettings.nbLines << 1;
    else
    	LPCmodSettings.LCDsettings.nbLines = 4;

    BootLCDSwitchType();
    initialLCDPrint();
    sprintf(itemStr,"%u", LPCmodSettings.LCDsettings.nbLines);
}

void LCDDecreaseLineLength(void * itemStr){
    if(LPCmodSettings.LCDsettings.lineLength > 8)
        LPCmodSettings.LCDsettings.lineLength -= 1;
    else
        LPCmodSettings.LCDsettings.lineLength = 8;

    BootLCDSwitchType();
    initialLCDPrint();
    sprintf(itemStr,"%u", LPCmodSettings.LCDsettings.lineLength);
}

void LCDIncreaseLineLength(void * itemStr){
    if(LPCmodSettings.LCDsettings.lineLength < 20)
        LPCmodSettings.LCDsettings.lineLength += 1;
    else
        LPCmodSettings.LCDsettings.lineLength = 20;

    BootLCDSwitchType();
    initialLCDPrint();
    sprintf(itemStr,"%u", LPCmodSettings.LCDsettings.lineLength);
}
