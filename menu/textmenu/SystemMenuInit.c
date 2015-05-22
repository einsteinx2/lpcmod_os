/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "include/boot.h"
#include "BootIde.h"
#include "TextMenu.h"
#include "SystemMenuActions.h"

TEXTMENU *SystemMenuInit(void) {
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;

    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "System settings");

    //LED SETTINGS MENU
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"LED");
    itemPtr->szParameter[0]=0;
    itemPtr->functionPtr=DrawChildTextMenu;
    itemPtr->functionDataPtr = (void *)LEDMenuInit();
    TextMenuAddItem(menuPtr, itemPtr);
    
    //FAN SPEED
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption,"Fan speed : ");
    sprintf(itemPtr->szParameter, "%d%%", LPCmodSettings.OSsettings.fanSpeed);
    itemPtr->functionPtr=NULL;
    itemPtr->functionDataPtr = NULL;
    itemPtr->functionLeftPtr=decrementFanSpeed;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=incrementFanSpeed;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    itemPtr->functionLTPtr=decrementFanSpeed;
    itemPtr->functionLTDataPtr = itemPtr->szParameter;
    itemPtr->functionRTPtr=incrementFanSpeed;
    itemPtr->functionRTDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    //VIDEO SETTINGS MENU
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Video settings");
    itemPtr->szParameter[0]=0;
    itemPtr->functionPtr=DrawChildTextMenu;
    itemPtr->functionDataPtr = (void *)VideoMenuInit();
    TextMenuAddItem(menuPtr, itemPtr);

    //VIDEO SETTINGS MENU
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Network settings");
    itemPtr->szParameter[0]=0;
    itemPtr->functionPtr=DrawChildTextMenu;
    itemPtr->functionDataPtr = (void *)NetworkMenuInit();
    TextMenuAddItem(menuPtr, itemPtr);

    //DVD REGION SETTINGS MENU
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "DVD region : ");
    sprintf(itemPtr->szParameter, "%s", DVDregiontext[eeprom.DVDPlaybackKitZone[0]]);
    itemPtr->functionPtr= NULL;
    itemPtr->functionDataPtr = NULL;
    itemPtr->functionLeftPtr=decrementDVDRegion;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=incrementDVDRegion;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    //GAME REGION SETTINGS MENU
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Game region : ");
    sprintf(itemPtr->szParameter, "%s", Gameregiontext[getGameRegionValue(&eeprom)]);
    itemPtr->functionPtr= NULL;
    itemPtr->functionDataPtr = NULL;
    itemPtr->functionLeftPtr=decrementGameRegion;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=incrementGameRegion;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    return menuPtr;
}
