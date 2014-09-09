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
    char *Gameregiontext[5] = {
            "Unknown/Error",
            "NTSC-U",
            "NTSC-J",
            "n/a",
            "PAL"
        };
    if(ConfirmDialog("         Confirm change Game region?", 0))
        return;
        
    int gameRegion = getGameRegionValue();
    switch(gameRegion){
        case NORTH_AMERICA:
            gameRegion = setGameRegionValue(JAPAN);
            break;
        case JAPAN:
            gameRegion = setGameRegionValue(EURO_AUSTRALIA);
            break;
        case EURO_AUSTRALIA:
            gameRegion = setGameRegionValue(NORTH_AMERICA);
            break;
        default:
            break;
        }
    sprintf(itemStr, "%s",Gameregiontext[gameRegion]);
}

void decrementGameRegion(void * itemStr){
    char *Gameregiontext[5] = {
            "Unknown/Error",
            "NTSC-U",
            "NTSC-J",
            "n/a",
            "PAL"
        };
    if(ConfirmDialog("         Confirm change Game region?",0))
        return;
        
    int gameRegion = getGameRegionValue();
    switch(gameRegion){
        case NORTH_AMERICA:
            gameRegion = setGameRegionValue(EURO_AUSTRALIA);
            break;
        case JAPAN:
            gameRegion = setGameRegionValue(NORTH_AMERICA);
            break;
        case EURO_AUSTRALIA:
            gameRegion = setGameRegionValue(JAPAN);
            break;
        }
    sprintf(itemStr, "%s",Gameregiontext[gameRegion]);
}

void incrementDVDRegion(void * itemStr){
    //String enum for DVD_ZONE
    char *DVDregiontext[9] = {
        "Region Clear",
        "USA (1)",
        "Europe (2)",
        "India (3)",
        "Australia (4)",
        "USSR (5)",
        "China (6)",
        "Free (7)",
        "Airlines (8)"
    };
    if(eeprom.DVDPlaybackKitZone[0] < 8)
        eeprom.DVDPlaybackKitZone[0] += 1;
    sprintf(itemStr, "%s",DVDregiontext[eeprom.DVDPlaybackKitZone[0]]);
    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}

void decrementDVDRegion(void * itemStr){
    //String enum for DVD_ZONE
    char *DVDregiontext[9] = {
        "Region Clear",
        "USA (1)",
        "Europe (2)",
        "India (3)",
        "Australia (4)",
        "USSR (5)",
        "China (6)",
        "Free (7)",
        "Airlines (8)"
    };
    if(eeprom.DVDPlaybackKitZone[0]  > 0)
        eeprom.DVDPlaybackKitZone[0] -= 1;
    sprintf(itemStr, "%s",DVDregiontext[eeprom.DVDPlaybackKitZone[0]]);
    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}

void incrementVideoformat(void * itemStr){
    char *VideoFormattext[17] = {
        "Full screen",
        "Widescreen",
        //not used
        "0x02",
        "0x03",
        "0x04",
        "0x05",
        "0x06",
        "0x07",
        "0x08",
        "0x09",
        "0x0A",
        "0x0B",
        "0x0C",
        "0x0D",
        "0x0E",
        "0x0F",
        //just easier to manage that way.
        "Letterbox"
    };
    switch(eeprom.VideoFlags[2]) {
        case FULLSCREEN:
            eeprom.VideoFlags[2] = WIDESCREEN;
            break;
        case WIDESCREEN:
            eeprom.VideoFlags[2] = LETTERBOX;
            break;
        case LETTERBOX:
            eeprom.VideoFlags[2] = FULLSCREEN;
            break;
        default:
        eeprom.VideoFlags[2] = FULLSCREEN;
            break;
    }
    sprintf(itemStr, "%s", VideoFormattext[eeprom.VideoFlags[2]]);
    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}

void decrementVideoformat(void * itemStr){
    char *VideoFormattext[17] = {
        "Full screen",
        "Widescreen",
        //not used
        "0x02",
        "0x03",
        "0x04",
        "0x05",
        "0x06",
        "0x07",
        "0x08",
        "0x09",
        "0x0A",
        "0x0B",
        "0x0C",
        "0x0D",
        "0x0E",
        "0x0F",
        //just easier to manage that way.
        "Letterbox"
    };
    switch(eeprom.VideoFlags[2]) {
        case FULLSCREEN:
            eeprom.VideoFlags[2] = LETTERBOX;
            break;
        case WIDESCREEN:
            eeprom.VideoFlags[2] = FULLSCREEN;
            break;
        case LETTERBOX:
            eeprom.VideoFlags[2] = WIDESCREEN;
            break;
        default:
        eeprom.VideoFlags[2] = FULLSCREEN;
            break;
    }
    sprintf(itemStr, "%s", VideoFormattext[eeprom.VideoFlags[2]]);
    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}
