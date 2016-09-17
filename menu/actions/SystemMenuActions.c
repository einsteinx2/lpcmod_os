/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "SystemMenuActions.h"
#include "boot.h"
#include "i2c.h"
#include "video.h"
#include "string.h"
#include "xblast/settings/xblastSettingsDefs.h"
#include "menu/misc/ConfirmDialog.h"

void toggleBGColor(void * itemStr)
{
	if(LPCmodSettings.OSsettings.backgroundColorPreset == 0)
	{
		LPCmodSettings.OSsettings.backgroundColorPreset = 1;
	}
	else
	{
		LPCmodSettings.OSsettings.backgroundColorPreset = 0;
	}

	BootVideoInitJPEGBackdropBuffer(&jpegBackdrop);
	//Screen will be refreched in TextMenu main routine.
	bgColorString((char *)itemStr);

	//Refresh IconMenu too
	refreshIconMenu = 1;
}

void bgColorString(char * stringOut)
{
	//String enum for BACKGROUND COLOR
	static const char * const BackgroundColor[] = {
	    "Blue",
	    "Purple"
	};

	if(stringOut == NULL)
	{
		return;
	}

	sprintf(stringOut, "%s", BackgroundColor[LPCmodSettings.OSsettings.backgroundColorPreset]);
}

void incrementFanSpeed(void * itemStr){
    if(LPCmodSettings.OSsettings.fanSpeed < 99)    //Logic
        LPCmodSettings.OSsettings.fanSpeed += 2;    //Actual value range of PIC is from 0 to 50 which maps to 0 to 100 here.
    if(LPCmodSettings.OSsettings.fanSpeed % 2)        //Failsafe in the near-impossible event that the value of fanSpeed would be odd.
        LPCmodSettings.OSsettings.fanSpeed -= 1;
    sprintf(itemStr, "%d%%", LPCmodSettings.OSsettings.fanSpeed);
    I2CSetFanSpeed(LPCmodSettings.OSsettings.fanSpeed);    //Send new speed to PIC
}

void decrementFanSpeed(void * itemStr){
    if(LPCmodSettings.OSsettings.fanSpeed > 11)    //For now, I won't allow anyone to set their fan below 10% speed. Come on...
        LPCmodSettings.OSsettings.fanSpeed -= 2;
    if(LPCmodSettings.OSsettings.fanSpeed % 2)        //Failsafe in the near-impossible event that the value of fanSpeed would be odd.
        LPCmodSettings.OSsettings.fanSpeed += 1;
    sprintf(itemStr, "%d%%", LPCmodSettings.OSsettings.fanSpeed);
    I2CSetFanSpeed(LPCmodSettings.OSsettings.fanSpeed);    //Send new speed to PIC
}

void incrementGameRegion(void * itemStr){

    if(ConfirmDialog("           Confirm change Game region?", 0))
        return;
        
    int gameRegion = getGameRegionValue(&eeprom);
    switch(gameRegion){
        case NORTH_AMERICA:
            gameRegion = setGameRegionValue(JAPAN);
            break;
        case JAPAN:
            gameRegion = setGameRegionValue(EURO_AUSTRALIA);
            break;
        case EURO_AUSTRALIA:
        default:
            gameRegion = setGameRegionValue(NORTH_AMERICA);
            break;
        }
    sprintf(itemStr, "%s",Gameregiontext[gameRegion]);
}

void decrementGameRegion(void * itemStr){

    if(ConfirmDialog("           Confirm change Game region?",0))
        return;
        
    int gameRegion = getGameRegionValue(&eeprom);
    switch(gameRegion){
        case NORTH_AMERICA:
            gameRegion = setGameRegionValue(EURO_AUSTRALIA);
            break;
        case EURO_AUSTRALIA:
            gameRegion = setGameRegionValue(JAPAN);
            break;
        case JAPAN:
        default:
            gameRegion = setGameRegionValue(NORTH_AMERICA);
            break;
        }
    sprintf(itemStr, "%s",Gameregiontext[gameRegion]);
}

void incrementDVDRegion(void * itemStr){
    if(eeprom.DVDPlaybackKitZone[0] < 8)
        eeprom.DVDPlaybackKitZone[0] += 1;
    sprintf(itemStr, "%s",DVDregiontext[eeprom.DVDPlaybackKitZone[0]]);
    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}

void decrementDVDRegion(void * itemStr){
    if(eeprom.DVDPlaybackKitZone[0]  > 0)
        eeprom.DVDPlaybackKitZone[0] -= 1;
    sprintf(itemStr, "%s",DVDregiontext[eeprom.DVDPlaybackKitZone[0]]);
    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}
