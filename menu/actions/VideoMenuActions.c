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
#include "VideoMenuActions.h"
#include "VideoInitialization.h"
#include "BootEEPROM.h"
#include "lib/LPCMod/xblastDebug.h"
#include "string.h"

void incrementVideoStandard(void * itemStr)
{
    EEPROM_VideoStandard newValue = *((EEPROM_VideoStandard *)&eeprom.VideoStandard);

    switch(newValue)
    {
        case EEPROM_VideoStandardNTSC_M:
            newValue = EEPROM_VideoStandardNTSC_J;
            break;
        case EEPROM_VideoStandardNTSC_J:
            newValue = EEPROM_VideoStandardPAL_I;
            break;
        case EEPROM_VideoStandardPAL_I:
            /* Fall through */
        default:
            newValue = EEPROM_VideoStandardNTSC_M;
        break;
    }

    EepromSetVideoStandard(newValue);
    strcpy(itemStr, getVideoStandardText(newValue));
}

void decrementVideoStandard(void * itemStr)
{
    EEPROM_VideoStandard newValue = *((EEPROM_VideoStandard *)&eeprom.VideoStandard);


    switch(newValue)
    {
        case EEPROM_VideoStandardNTSC_M:
            newValue = EEPROM_VideoStandardPAL_I;
            break;
        case EEPROM_VideoStandardPAL_I:
            newValue = EEPROM_VideoStandardNTSC_J;
            break;
        case EEPROM_VideoStandardNTSC_J:
            /* Fall through */
        default:
            newValue = EEPROM_VideoStandardNTSC_M;
        break;
    }

    EepromSetVideoStandard(newValue);
    strcpy(itemStr, getVideoStandardText(newValue));
}

void incrementVideoformat(void * itemStr)
{
    EEPROM_VidScreenFormat format;

    if(eeprom.VideoFlags[2] & EEPROM_VidScreenWidescreen)              //Set to Letterbox
    {
        format = EEPROM_VidScreenLetterbox;
    }
    else if(eeprom.VideoFlags[2] & EEPROM_VidScreenLetterbox)           //Set to Fullscreen
    {
        format = EEPROM_VidScreenFullScreen;
    }
    else
    {
        format = EEPROM_VidScreenWidescreen;
    }
    EepromSetVideoFormat(format);

    strcpy(itemStr, getScreenFormatText(format));
}

void decrementVideoformat(void * itemStr)
{
    EEPROM_VidScreenFormat format;

    if(eeprom.VideoFlags[2] & EEPROM_VidScreenWidescreen)              //Set to Fullscreen
    {
        format = EEPROM_VidScreenFullScreen;
    }
    else if(eeprom.VideoFlags[2] & EEPROM_VidScreenLetterbox)           //Set to Widescreen
    {
        format = EEPROM_VidScreenWidescreen;
    }
    else
    {
        format = EEPROM_VidScreenLetterbox;                             //Set to Letterbox
    }
    EepromSetVideoFormat(format);
    strcpy(itemStr, getScreenFormatText(format));
}

void toggle480p(void * itemStr)
{
    if (eeprom.VideoFlags[2] & EEPROM_VidResolutionEnable480p)         //480p already enabled?
    {
        eeprom.VideoFlags[2] &= ~EEPROM_VidResolutionEnable480p;       //Disable
        strcpy(itemStr, "No");
    }
    else
    {
        eeprom.VideoFlags[2] |= EEPROM_VidResolutionEnable480p;
        strcpy(itemStr, "Yes");
    }
    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}

void toggle720p(void * itemStr)
{
    if (eeprom.VideoFlags[2] & EEPROM_VidResolutionEnable720p)         //720p already enabled?
    {
        eeprom.VideoFlags[2] &= ~EEPROM_VidResolutionEnable720p;       //Disable
        strcpy(itemStr, "No");
    }
    else
    {
        eeprom.VideoFlags[2] |= EEPROM_VidResolutionEnable720p;
        strcpy(itemStr, "Yes");
    }
    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}

void toggle1080i(void * itemStr)
{
    if (eeprom.VideoFlags[2] & EEPROM_VidResolutionEnable1080i)         //1080i already enabled?
    {
        eeprom.VideoFlags[2] &= ~EEPROM_VidResolutionEnable1080i;       //Disable
        strcpy(itemStr, "No");
    }
    else
    {
        eeprom.VideoFlags[2] |= EEPROM_VidResolutionEnable1080i;
        strcpy(itemStr, "Yes");
    }
    EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}

void toggleVGA(void* itemStr)
{
    if(LPCmodSettings.OSsettings.enableVGA)
    {
        LPCmodSettings.OSsettings.enableVGA = 0;
    }
    else
    {
        LPCmodSettings.OSsettings.enableVGA = 1;
    }

    BootVgaInitializationKernelNG((CURRENT_VIDEO_MODE_DETAILS *)&vmode);

    strcpy(itemStr, LPCmodSettings.OSsettings.enableVGA? "Yes" : "No");
}
