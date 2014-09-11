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

void LCDToggleEN5V(void * itemStr){
    LPCmodSettings.LCDsettings.enable5V = LPCmodSettings.LCDsettings.enable5V? 0 : 1;
    sprintf(itemStr,"%s", LPCmodSettings.LCDsettings.enable5V? "Yes" : "No");
    assertInitLCD();
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
    sprintf(itemStr,"%s", LPCmodSettings.LCDsettings.displayMsgBoot? "Yes" : "No");
}

void LCDToggledisplayBIOSNameBoot(void * itemStr){
    LPCmodSettings.LCDsettings.displayBIOSNameBoot = LPCmodSettings.LCDsettings.displayBIOSNameBoot? 0 : 1;
    sprintf(itemStr,"%s", LPCmodSettings.LCDsettings.displayBIOSNameBoot? "Yes" : "No");
}

void LCDToggledisplayCustomTextBoot(void * itemStr){
    LPCmodSettings.LCDsettings.customTextBoot = LPCmodSettings.LCDsettings.customTextBoot? 0 : 1;
    sprintf(itemStr,"%s", LPCmodSettings.LCDsettings.customTextBoot? "Yes" : "No");
    initialLCDPrint();
}

void editCustomString0(void *whatever){
    OnScreenKeyboard(LPCmodSettings.LCDsettings.customString0, LPCmodSettings.LCDsettings.lineLength);
}

void editCustomString1(void *whatever){
    OnScreenKeyboard(LPCmodSettings.LCDsettings.customString1, LPCmodSettings.LCDsettings.lineLength);
}

void editCustomString2(void *whatever){
    OnScreenKeyboard(LPCmodSettings.LCDsettings.customString2, LPCmodSettings.LCDsettings.lineLength);
}

void editCustomString3(void *whatever){
    OnScreenKeyboard(LPCmodSettings.LCDsettings.customString3, LPCmodSettings.LCDsettings.lineLength);
}
