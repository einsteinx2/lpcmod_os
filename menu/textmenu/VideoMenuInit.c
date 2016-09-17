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
#include "VideoMenuActions.h"
#include "VideoInitialization.h"
#include "string.h"

TEXTMENU *VideoMenuInit(void) {
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;

    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "Video Settings Menu");

//    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
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
    
    
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "TV Standard : ");
    switch(*((VIDEO_STANDARD *)&eeprom.VideoStandard)) {
        case NTSC_M:
            sprintf(itemPtr->szParameter, "%s", "NTSC-U");
            break;
        case NTSC_J:
            sprintf(itemPtr->szParameter, "%s", "NTSC-J");
            break;
        case PAL_I:
            sprintf(itemPtr->szParameter, "%s", "PAL");
            break;
        default:
            sprintf(itemPtr->szParameter, "%s", "Unknown");
        break;
    }
    itemPtr->functionPtr=incrementVideoStandard;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr=decrementVideoStandard;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=incrementVideoStandard;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    //VIDEO FORMAT SETTINGS MENU
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Video format : ");
    if(eeprom.VideoFlags[2] & WIDESCREEN){
        sprintf(itemPtr->szParameter, "%s", "Widescreen");
    }
    else {
        if(eeprom.VideoFlags[2] & LETTERBOX){
            sprintf(itemPtr->szParameter, "%s", "Letterbox");
        }
        else{
            sprintf(itemPtr->szParameter, "%s", "Fullscreen");
        }
    }
    itemPtr->functionPtr= incrementVideoformat;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr=decrementVideoformat;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=incrementVideoformat;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Enable 480p : ");
    sprintf(itemPtr->szParameter, "%s", (eeprom.VideoFlags[2] & R480p)? "Yes" : "No");
    itemPtr->functionPtr=toggle480p;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr=toggle480p;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=toggle480p;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Enable 7200p : ");
    sprintf(itemPtr->szParameter, "%s", (eeprom.VideoFlags[2] & R720p)? "Yes" : "No");
    itemPtr->functionPtr=toggle720p;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr=toggle720p;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=toggle720p;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Enable 1080i : ");
    sprintf(itemPtr->szParameter, "%s", (eeprom.VideoFlags[2] & R1080i)? "Yes" : "No");
    itemPtr->functionPtr=toggle1080i;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr=toggle1080i;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=toggle1080i;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    return menuPtr;
}
