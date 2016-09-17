/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "boot.h"
#include "video.h"
#include "VideoInitialization.h"
#include "BootEEPROM.h"
#include "string.h"

void incrementVideoStandard(void * itemStr){
    switch(*((VIDEO_STANDARD *)&eeprom.VideoStandard)) {
        case NTSC_M:
            sprintf(itemStr, "%s", "NTSC-J");
            EepromSetVideoStandard(NTSC_J);
            break;
        case NTSC_J:
            sprintf(itemStr, "%s", "PAL");
            EepromSetVideoStandard(PAL_I);
            break;
        case PAL_I:
            sprintf(itemStr, "%s", "NTSC-U");
            EepromSetVideoStandard(NTSC_M);
            break;
        default:
            sprintf(itemStr, "%s", "NTSC-U");
            EepromSetVideoStandard(NTSC_M);
        break;
    }
}

void decrementVideoStandard(void * itemStr){
    switch(*((VIDEO_STANDARD *)&eeprom.VideoStandard)) {
        case NTSC_M:
            sprintf(itemStr, "%s", "PAL");
            EepromSetVideoStandard(PAL_I);
            break;
        case NTSC_J:
            sprintf(itemStr, "%s", "NTSC-U");
            EepromSetVideoStandard(NTSC_M);
            break;
        case PAL_I:
            sprintf(itemStr, "%s", "NTSC-J");
            EepromSetVideoStandard(NTSC_J);
            break;
        default:
            sprintf(itemStr, "%s", "NTSC-U");
            EepromSetVideoStandard(NTSC_M);
        break;
    }
}

void incrementVideoformat(void * itemStr){
    if(eeprom.VideoFlags[2] & WIDESCREEN){              //Set to Letterbox
        eeprom.VideoFlags[2] &= ~WIDESCREEN;            //Remove Widescreen bit
        eeprom.VideoFlags[2] |= LETTERBOX;
        sprintf(itemStr, "%s", "Letterbox");
    }
    else {
        if(eeprom.VideoFlags[2] & LETTERBOX){           //Set to Fullscreen
            eeprom.VideoFlags[2] &= ~LETTERBOX;
            sprintf(itemStr, "%s", "Fullscreen");
        }
        else{
            eeprom.VideoFlags[2] |= WIDESCREEN;
            sprintf(itemStr, "%s", "Widescreen");       //Set to Widescreen
        }
    }
    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}

void decrementVideoformat(void * itemStr){
    if(eeprom.VideoFlags[2] & WIDESCREEN){              //Set to Fullscreen
        eeprom.VideoFlags[2] &= ~WIDESCREEN;            //Remove Widescreen bit
        sprintf(itemStr, "%s", "Fullscreen");
    }
    else {
        if(eeprom.VideoFlags[2] & LETTERBOX){           //Set to Widescreen
            eeprom.VideoFlags[2] &= ~LETTERBOX;
            eeprom.VideoFlags[2] |= WIDESCREEN;
            sprintf(itemStr, "%s", "Widescreen");
        }
        else{
            eeprom.VideoFlags[2] |= LETTERBOX;
            sprintf(itemStr, "%s", "Letterbox");        //Set to Letterbox
        }
    }
    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}

void toggle480p(void * itemStr){
    if (eeprom.VideoFlags[2] & R480p){         //480p already enabled?
        eeprom.VideoFlags[2] &= ~R480p;        //Disable
        sprintf(itemStr, "%s", "No");
    }
    else {
        eeprom.VideoFlags[2] |= R480p;
        sprintf(itemStr, "%s", "Yes");
    }
    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}

void toggle720p(void * itemStr){
    if (eeprom.VideoFlags[2] & R720p){         //720p already enabled?
        eeprom.VideoFlags[2] &= ~R720p;        //Disable
        sprintf(itemStr, "%s", "No");
    }
    else {
        eeprom.VideoFlags[2] |= R720p;
        sprintf(itemStr, "%s", "Yes");
    }
    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}

void toggle1080i(void * itemStr){
    if (eeprom.VideoFlags[2] & R1080i){         //1080i already enabled?
        eeprom.VideoFlags[2] &= ~R1080i;        //Disable
        sprintf(itemStr, "%s", "No");
    }
    else {
        eeprom.VideoFlags[2] |= R1080i;
        sprintf(itemStr, "%s", "Yes");
    }
    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}
