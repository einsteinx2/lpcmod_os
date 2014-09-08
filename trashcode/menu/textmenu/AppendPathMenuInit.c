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
#include "AppendPathMenuActions.h"

TEXTMENU *AppendPathMenuInit(void) {
	
	TEXTMENUITEM *itemPtr;
	TEXTMENU *menuPtr;

	extern char appendPath[200];
	memset(appendPath, 0, 200);

	//Create the root menu - MANDATORY
	menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
   memset(menuPtr,0x00,sizeof(TEXTMENU));
	menuPtr->timeout = 1;
	menuPtr->longTitle = 1;
	menuPtr->visibleCount = 3;
	strcpy(menuPtr->szCaption, "/append.txt");
	menuPtr->firstMenuItem=NULL;

	strcpy(appendPath, menuPtr->szCaption);


	// Connect
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Connect");
	itemPtr->functionPtr=enableHttpcAppendPath;
	itemPtr->functionDataPtr = NULL ;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetAppendPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetAppendPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);
	
	// New letter
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "->");
	itemPtr->functionPtr=nextLetterAppendPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetAppendPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetAppendPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Delete
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "<-");
	itemPtr->functionPtr=deleteLetterAppendPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetAppendPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetAppendPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Uppercase
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Uppercase");
	itemPtr->functionPtr=setUCAppendPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetAppendPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetAppendPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Lowercase
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Lowercase");
	itemPtr->functionPtr=setLCAppendPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetAppendPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetAppendPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Number
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Numbers");
	itemPtr->functionPtr=setNumAppendPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetAppendPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetAppendPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Fullstop (.)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Fullstop (.)");
	itemPtr->functionPtr=setFullStopAppendPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetAppendPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetAppendPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Forward slash (/)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Forward slash (/)");
	itemPtr->functionPtr=setFSlashAppendPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetAppendPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetAppendPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Dash (-)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Dash (-)");
	itemPtr->functionPtr=setDashAppendPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetAppendPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetAppendPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Underscore (_)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Underscore (_)");
	itemPtr->functionPtr=setUScoreAppendPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetAppendPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetAppendPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	return menuPtr;
}
