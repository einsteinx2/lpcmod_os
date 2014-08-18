/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "TextMenu.h"
#include "ToolsMenuActions.h"
#include "lpcmod_v1.h"

TEXTMENU *ToolsMenuInit(void) {
	TEXTMENUITEM *itemPtr;
	TEXTMENU *menuPtr;
	bool fHasHardware=false;
	int i=0;

	//No entry in this menu will have a configurable parameter.
	//Set first character to NULL to indicate no string is to be shown.
	itemPtr->szParameter[0]=0;

	menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
	memset(menuPtr,0x00,sizeof(TEXTMENU));
	strcpy(menuPtr->szCaption, "Tools");

	if(LPCMod_HW_rev() == SYSCON_ID)
				fHasHardware = true;

	if(fHasHardware) {
		//Save EEPROM data to flash
		itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
		memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
		sprintf(itemPtr->szCaption,"Save EEPROM to modchip");
		itemPtr->functionPtr= saveEEPromToFlash;
		itemPtr->functionDataPtr = NULL;
		TextMenuAddItem(menuPtr, itemPtr);

		//Restore EEPROM data from flash
		itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
		memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
		strcpy(itemPtr->szCaption, "Restore EEPROM from modchip");
		itemPtr->functionPtr= restoreEEPromFromFlash;
		itemPtr->functionDataPtr = NULL;
		TextMenuAddItem(menuPtr, itemPtr);
	}

	//Wipe EEPROM section that holds non-vital data.
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Reset system settings");
	itemPtr->functionPtr= wipeEEPromUserSettings;
	itemPtr->functionDataPtr = NULL;
	TextMenuAddItem(menuPtr, itemPtr);

	//128MB MEMORY TEST
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "128MB RAM test");
	itemPtr->functionPtr= memtest;
	itemPtr->functionDataPtr = NULL;
	TextMenuAddItem(menuPtr, itemPtr);

	return menuPtr;
}
