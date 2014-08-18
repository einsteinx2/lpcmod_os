/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "include/config.h"
#include "TextMenu.h"
#include "lpcmod_v1.h"
#include "VideoInitialization.h"

TEXTMENU *TextMenuInit(void) {
	
	TEXTMENUITEM *itemPtr;
	TEXTMENU *menuPtr;
	bool fHasHardware=false;
	
	//Create the root menu - MANDATORY
	menuPtr = malloc(sizeof(TEXTMENU));
	memset(menuPtr,0x00,sizeof(TEXTMENU));
	strcpy(menuPtr->szCaption, "LPCMod v1 configuration menu");
	menuPtr->firstMenuItem=NULL;

	//No entry in this menu will have a configurable parameter.
	//Set first character to NULL to indicate no string is to be shown.
	itemPtr->szParameter[0]=0;

	if(LPCMod_HW_rev() == SYSCON_ID)
		fHasHardware = true;
	
	if(fHasHardware) {							//No need to display this menu if no modchip is present.
		//LPCMod(modchip) SETTINGS MENU
		itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
		memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
		strcpy(itemPtr->szCaption, "LPCMod settings");
		itemPtr->functionPtr=DrawChildTextMenu;
		itemPtr->functionDataPtr = (void *)ModchipMenuInit();
		TextMenuAddItem(menuPtr, itemPtr);
	}

	//SYSTEM SETTINGS MENU
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "System settings");
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)SystemMenuInit();
	TextMenuAddItem(menuPtr, itemPtr);

	if(fHasHardware) {							//No need to display this menu if no modchip is present.
		//LCD SETTINGS MENU
		itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
		memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
		strcpy(itemPtr->szCaption, "LCD settings");
		itemPtr->functionPtr=DrawChildTextMenu;
		itemPtr->functionDataPtr = (void *)LCDMenuInit();
		TextMenuAddItem(menuPtr, itemPtr);
	}

	//TOOLS MENU
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Tools");
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)ToolsMenuInit();
	TextMenuAddItem(menuPtr, itemPtr);


#ifdef FLASH
	//FLASH MENU
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Flash menu");
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)BankSelectMenuInit();
	TextMenuAddItem(menuPtr, itemPtr);
#endif

	//HDD MENU
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "HDD menu");
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)HDDMenuInit();
	TextMenuAddItem(menuPtr, itemPtr);

	//CD MENU
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "CD menu");
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)CDMenuInit();
	TextMenuAddItem(menuPtr, itemPtr);

	// Info Menu
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Info menu");
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)InfoMenuInit();
	TextMenuAddItem(menuPtr, itemPtr);

	// Power Menu
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Power menu");
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)ResetMenuInit();
	TextMenuAddItem(menuPtr, itemPtr);
	
	return menuPtr;
}
