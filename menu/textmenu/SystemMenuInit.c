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
#include "BootIde.h"
#include "SystemMenuActions.h"
#include "lpcmod_v1.h"
#include "string.h"
#include "stdio.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "xblast/HardwareIdentifier.h"

TEXTMENU *SystemMenuInit(void) {
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "System settings");

    if(fSpecialEdition == SYSCON_ID_V1_PRE_EDITION)
    {
		//BACKGROUND COLOR SETTINGS MENU
		itemPtr = malloc(sizeof(TEXTMENUITEM));
		memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
		strcpy(itemPtr->szCaption, "Background color : ");
		bgColorString(itemPtr->szParameter);
		itemPtr->functionPtr = NULL;
		itemPtr->functionDataPtr = NULL;
		itemPtr->functionLeftPtr=toggleBGColor;
		itemPtr->functionLeftDataPtr = itemPtr->szParameter;
		itemPtr->functionRightPtr=toggleBGColor;
		itemPtr->functionRightDataPtr = itemPtr->szParameter;
		TextMenuAddItem(menuPtr, itemPtr);
    }

    //LED SETTINGS MENU
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption,"LED");
    itemPtr->szParameter[0]=0;
    itemPtr->functionPtr=DrawChildTextMenu;
    itemPtr->functionDataPtr = LEDMenuInit();
    TextMenuAddItem(menuPtr, itemPtr);
    
    //FAN SPEED
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
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
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Video settings");
    itemPtr->szParameter[0]=0;
    itemPtr->functionPtr=DrawChildTextMenu;
    itemPtr->functionDataPtr = VideoMenuInit();
    TextMenuAddItem(menuPtr, itemPtr);

    //VIDEO SETTINGS MENU
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Network settings");
    itemPtr->szParameter[0]=0;
    itemPtr->functionPtr=DrawChildTextMenu;
    itemPtr->functionDataPtr = NetworkMenuInit();
    TextMenuAddItem(menuPtr, itemPtr);

    //DVD REGION SETTINGS MENU
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "DVD region : ");
    strcpy(itemPtr->szParameter, getDVDRegionText(eeprom.DVDPlaybackKitZone[0]));
    itemPtr->functionPtr = NULL;
    itemPtr->functionDataPtr = NULL;
    itemPtr->functionLeftPtr=decrementDVDRegion;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=incrementDVDRegion;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    //GAME REGION SETTINGS MENU
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Game region : ");
    strcpy(itemPtr->szParameter, getGameRegionText(getGameRegionValue(&eeprom)));
    itemPtr->functionPtr = NULL;
    itemPtr->functionDataPtr = NULL;
    itemPtr->functionLeftPtr=decrementGameRegion;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=incrementGameRegion;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    return menuPtr;
}
