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

	//I know, I know... There are multiple definitions of these in the code. I don't care. It's not like they are going to change much.
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
	char *Gameregiontext[5] = {
		"Unknown/Error",
		"NTSC-U",
		"NTSC-J",
		"n/a",
		"PAL"
	};
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
	TextMenuAddItem(menuPtr, itemPtr);

	//VIDEO STANDARD SETTINGS MENU
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Video standard");
	itemPtr->szParameter[0]=0;
	itemPtr->szParameter[0]=0;
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)VideoMenuInit();
	TextMenuAddItem(menuPtr, itemPtr);

	//VIDEO FORMAT SETTINGS MENU
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Video format : ");
	sprintf(itemPtr->szParameter, "%s", VideoFormattext[eeprom.VideoFlags[2]]);
	itemPtr->functionPtr= incrementVideoformat;
	itemPtr->functionDataPtr = itemPtr->szParameter;
	itemPtr->functionLeftPtr=decrementVideoformat;
	itemPtr->functionLeftDataPtr = itemPtr->szParameter;
	itemPtr->functionRightPtr=incrementVideoformat;
	itemPtr->functionRightDataPtr = itemPtr->szParameter;
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
	sprintf(itemPtr->szParameter, "%s", Gameregiontext[getGameRegionValue()]);
	itemPtr->functionPtr= NULL;
	itemPtr->functionDataPtr = NULL;
	itemPtr->functionLeftPtr=decrementGameRegion;
	itemPtr->functionLeftDataPtr = itemPtr->szParameter;
	itemPtr->functionRightPtr=incrementGameRegion;
	itemPtr->functionRightDataPtr = itemPtr->szParameter;
	TextMenuAddItem(menuPtr, itemPtr);

	return menuPtr;
}
