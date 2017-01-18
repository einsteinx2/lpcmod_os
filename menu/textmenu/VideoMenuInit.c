/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "MenuInits.h"
#include "boot.h"
#include "xblast/HardwareIdentifier.h"
#include "VideoMenuActions.h"
#include "VideoInitialization.h"
#include "string.h"

TEXTMENU *VideoMenuInit(void)
{
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "Video Settings Menu");

//    itemPtr = malloc(sizeof(TEXTMENUITEM));
//    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
//    if(((unsigned char *)&eeprom)[0x96]&0x01) {
//        strcpy(itemPtr->szCaption, "Display Size: Widescreen");
//    }
//    else {
//        strcpy(itemPtr->szCaption, "Display Size: Normal");
//    }
//    itemPtr->functionPtr=SetWidescreen;
//    itemPtr->functionDataPtr = itemPtr->szCaption;
//    TextMenuAddItem(menuPtr, itemPtr);
    
    
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "TV Standard : ");
    sprintf(itemPtr->szParameter, "%s", getVideoStandardText(*((EEPROM_VideoStandard *)&eeprom.VideoStandard)));
    itemPtr->functionPtr=incrementVideoStandard;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr=decrementVideoStandard;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=incrementVideoStandard;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    //VIDEO FORMAT SETTINGS MENU
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Video format : ");
    EEPROM_VidScreenFormat format;
    if(eeprom.VideoFlags[2] & EEPROM_VidScreenWidescreen)
    {
        format = EEPROM_VidScreenWidescreen;
    }
    else if(eeprom.VideoFlags[2] & EEPROM_VidScreenLetterbox)
    {
        format = EEPROM_VidScreenLetterbox;
    }
    else
    {
        format = EEPROM_VidScreenFullScreen;
    }
    sprintf(itemPtr->szParameter, "%s", getScreenFormatText(format));
    itemPtr->functionPtr = incrementVideoformat;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr=decrementVideoformat;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=incrementVideoformat;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Enable 480p : ");
    sprintf(itemPtr->szParameter, "%s", (eeprom.VideoFlags[2] & EEPROM_VidResolutionEnable480p)? "Yes" : "No");
    itemPtr->functionPtr=toggle480p;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr=toggle480p;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=toggle480p;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Enable 720p : ");
    sprintf(itemPtr->szParameter, "%s", (eeprom.VideoFlags[2] & EEPROM_VidResolutionEnable720p)? "Yes" : "No");
    itemPtr->functionPtr=toggle720p;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr=toggle720p;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=toggle720p;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Enable 1080i : ");
    sprintf(itemPtr->szParameter, "%s", (eeprom.VideoFlags[2] & EEPROM_VidResolutionEnable1080i)? "Yes" : "No");
    itemPtr->functionPtr=toggle1080i;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr=toggle1080i;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=toggle1080i;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    if(isFrostySupport())
    {
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Enable VGA : ");
        sprintf(itemPtr->szParameter, "%s", LPCmodSettings.OSsettings.enableVGA? "Yes" : "No");
        itemPtr->functionPtr=toggleVGA;
        itemPtr->functionDataPtr = itemPtr->szParameter;
        itemPtr->functionLeftPtr=toggleVGA;
        itemPtr->functionLeftDataPtr = itemPtr->szParameter;
        itemPtr->functionRightPtr=toggleVGA;
        itemPtr->functionRightDataPtr = itemPtr->szParameter;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    return menuPtr;
}
