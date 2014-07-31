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
#include "KernelPathMenuActions.h"

TEXTMENU *KernelPathMenuInit(void) {
	
	TEXTMENUITEM *itemPtr;
	TEXTMENU *menuPtr;

	extern char kernelPath[200];
	memset(kernelPath, 0, 200);

	//Create the root menu - MANDATORY
	menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
   memset(menuPtr,0x00,sizeof(TEXTMENU));
	menuPtr->timeout = 1;
	menuPtr->longTitle = 1;
	menuPtr->visibleCount = 3;
	strcpy(menuPtr->szCaption, "/vmlinuz");
	menuPtr->firstMenuItem=NULL;

	strcpy(kernelPath, menuPtr->szCaption);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Next ->");
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)InitrdPathMenuInit();
	itemPtr->functionLeftPtr=decrementAlphabetKernelPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetKernelPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// New letter
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "->");
	itemPtr->functionPtr=nextLetterKernelPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetKernelPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetKernelPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Delete
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "<-");
	itemPtr->functionPtr=deleteLetterKernelPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetKernelPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetKernelPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Uppercase
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Uppercase");
	itemPtr->functionPtr=setUCKernelPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetKernelPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetKernelPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Lowercase
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Lowercase");
	itemPtr->functionPtr=setLCKernelPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetKernelPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetKernelPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Number
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Numbers");
	itemPtr->functionPtr=setNumKernelPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetKernelPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetKernelPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Fullstop (.)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Fullstop (.)");
	itemPtr->functionPtr=setFullStopKernelPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetKernelPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetKernelPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Forward slash (/)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Forward slash (/)");
	itemPtr->functionPtr=setFSlashKernelPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetKernelPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetKernelPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Dash (-)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Dash (-)");
	itemPtr->functionPtr=setDashKernelPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetKernelPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetKernelPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	// Underscore (_)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	sprintf(itemPtr->szCaption, "Underscore (_)");
	itemPtr->functionPtr=setUScoreKernelPath;
	itemPtr->functionDataPtr = menuPtr->szCaption;
	itemPtr->functionLeftPtr=decrementAlphabetKernelPath;
	itemPtr->functionLeftDataPtr = menuPtr->szCaption;
	itemPtr->functionRightPtr=incrementAlphabetKernelPath;
	itemPtr->functionRightDataPtr = menuPtr->szCaption;
	TextMenuAddItem(menuPtr, itemPtr);

	return menuPtr;
}
