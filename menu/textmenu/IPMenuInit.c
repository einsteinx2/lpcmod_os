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
#include "IPMenuActions.h"

TEXTMENU *IPMenuInit(void) {
	
	TEXTMENUITEM *itemPtr;
	TEXTMENU *menuPtr;

	//Create the root menu - MANDATORY
	menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
   memset(menuPtr,0x00,sizeof(TEXTMENU));
	menuPtr->timeout = 1;
	strcpy(menuPtr->szCaption, "IP:Port Selection (A.B.C.D:P)");
	menuPtr->firstMenuItem=NULL;

	// Connect
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Next ->");
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)URLMenuInit();
	TextMenuAddItem(menuPtr, itemPtr);
	
	// A
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "%s %i", "A: ", WB_BLOCK_A);
	itemPtr->functionPtr=skipTenA;
	itemPtr->functionDataPtr = itemPtr->szCaption;
	itemPtr->functionLeftPtr=decrementNumberA;
	itemPtr->functionLeftDataPtr = itemPtr->szCaption;
	itemPtr->functionRightPtr=incrementNumberA;
	itemPtr->functionRightDataPtr = itemPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// B
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "%s %i", "B: ", WB_BLOCK_B);
	itemPtr->functionPtr=skipTenB;
	itemPtr->functionDataPtr = itemPtr->szCaption;
	itemPtr->functionLeftPtr=decrementNumberB;
	itemPtr->functionLeftDataPtr = itemPtr->szCaption;
	itemPtr->functionRightPtr=incrementNumberB;
	itemPtr->functionRightDataPtr = itemPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// C
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "%s %i", "C: ", WB_BLOCK_C);
	itemPtr->functionPtr=skipTenC;
	itemPtr->functionDataPtr = itemPtr->szCaption;
	itemPtr->functionLeftPtr=decrementNumberC;
	itemPtr->functionLeftDataPtr = itemPtr->szCaption;
	itemPtr->functionRightPtr=incrementNumberC;
	itemPtr->functionRightDataPtr = itemPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// D
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "%s %i", "D: ", WB_BLOCK_D);
	itemPtr->functionPtr=skipTenD;
	itemPtr->functionDataPtr = itemPtr->szCaption;
	itemPtr->functionLeftPtr=decrementNumberD;
	itemPtr->functionLeftDataPtr = itemPtr->szCaption;
	itemPtr->functionRightPtr=incrementNumberD;
	itemPtr->functionRightDataPtr = itemPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// P
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "%s %i", "P: ", WB_PORT);
	itemPtr->functionPtr=skipTenP;
	itemPtr->functionDataPtr = itemPtr->szCaption;
	itemPtr->functionLeftPtr=decrementNumberP;
	itemPtr->functionLeftDataPtr = itemPtr->szCaption;
	itemPtr->functionRightPtr=incrementNumberP;
	itemPtr->functionRightDataPtr = itemPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	return menuPtr;
}
