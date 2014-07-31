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

#include "VideoInitialization.h"
#include "InitrdPathMenuActions.h"

TEXTMENU *InitrdPathMenuInit(void) {
	
	TEXTMENUITEM *itemPtr;
	TEXTMENU *menuPtr;

	extern char initrdPath[200];
	memset(initrdPath, 0, 200);

	//Create the root menu - MANDATORY
	menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
   memset(menuPtr,0x00,sizeof(TEXTMENU));
	menuPtr->timeout = 1;
	menuPtr->longTitle = 1;
	menuPtr->visibleCount = 3;
	strcpy(menuPtr->szCaption, "/initrd.gz");
	menuPtr->firstMenuItem=NULL;

	strcpy(initrdPath, menuPtr->szCaption);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Next ->");
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)AppendPathMenuInit();
	itemPtr->functionLeftPtr=decrementAlphabetInitrdPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetInitrdPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// New letter
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "->");
	itemPtr->functionPtr=nextLetterInitrdPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetInitrdPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetInitrdPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Delete
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "<-");
	itemPtr->functionPtr=deleteLetterInitrdPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetInitrdPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetInitrdPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Uppercase
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Uppercase");
	itemPtr->functionPtr=setUCInitrdPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetInitrdPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetInitrdPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Lowercase
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Lowercase");
	itemPtr->functionPtr=setLCInitrdPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetInitrdPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetInitrdPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Number
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Numbers");
	itemPtr->functionPtr=setNumInitrdPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetInitrdPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetInitrdPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Fullstop (.)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Fullstop (.)");
	itemPtr->functionPtr=setFullStopInitrdPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetInitrdPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetInitrdPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Forward slash (/)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Forward slash (/)");
	itemPtr->functionPtr=setFSlashInitrdPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetInitrdPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetInitrdPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Dash (-)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Dash (-)");
	itemPtr->functionPtr=setDashInitrdPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetInitrdPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetInitrdPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Underscore (_)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Underscore (_)");
	itemPtr->functionPtr=setUScoreInitrdPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetInitrdPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetInitrdPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	return menuPtr;
}
