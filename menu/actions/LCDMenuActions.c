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
	sprintf(itemStr,"Enable LCD : %s", LPCmodSettings.LCDsettings.enable5V? "Yes" : "No");
	assertInitLCD();
}

void LCDIncrementBacklight(void * itemStr){
	if(LPCmodSettings.LCDsettings.backlight < 99)
		LPCmodSettings.LCDsettings.backlight += 1;
	sprintf(itemStr, "Backlight : %d%%", LPCmodSettings.LCDsettings.backlight);
	setLCDBacklight(LPCmodSettings.LCDsettings.backlight);
}

void LCDDecrementBacklight(void * itemStr){
	if(LPCmodSettings.LCDsettings.backlight > 0)
		LPCmodSettings.LCDsettings.backlight -= 1;
	sprintf(itemStr, "Backlight : %d%%", LPCmodSettings.LCDsettings.backlight);
	setLCDBacklight(LPCmodSettings.LCDsettings.backlight);
}

void LCDIncrementContrast(void * itemStr){
	if(LPCmodSettings.LCDsettings.contrast < 99)
		LPCmodSettings.LCDsettings.contrast += 1;
	sprintf(itemStr, "Contrast : %d%%", LPCmodSettings.LCDsettings.contrast);
	setLCDContrast(LPCmodSettings.LCDsettings.contrast);
}

void LCDDecrementContrast(void * itemStr){
	if(LPCmodSettings.LCDsettings.contrast > 0)
		LPCmodSettings.LCDsettings.contrast -= 1;
	sprintf(itemStr, "Contrast : %d%%", LPCmodSettings.LCDsettings.contrast);
	setLCDContrast(LPCmodSettings.LCDsettings.contrast);
}

void LCDToggleDisplayBootMsg(void * itemStr){
	LPCmodSettings.LCDsettings.displayMsgBoot? 0 : 1;
	sprintf(itemStr,"Display message at boot : %s", LPCmodSettings.LCDsettings.displayMsgBoot? "Yes" : "No");
}

void LCDToggledisplayBIOSNameBoot(void * itemStr){
	LPCmodSettings.LCDsettings.displayBIOSNameBoot? 0 : 1;
	sprintf(itemStr,"Display booting BIOS name : %s", LPCmodSettings.LCDsettings.displayBIOSNameBoot? "Yes" : "No");
}
