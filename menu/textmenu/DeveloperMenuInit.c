/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "TextMenu.h"
#include "DeveloperMenuActions.h"
#include "lpcmod_v1.h"

TEXTMENU *DeveloperMenuInit(void) {
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;


    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "Developer tools");

    //Write to LPC port
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Write LPC I/O");
    itemPtr->functionPtr= LPCIOWrite;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);
    
    //Read LPC port data.
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Read LPC I/O");
    itemPtr->functionPtr= LPCIORead;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    if(fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_V1_TSOP){
        //Read GPI/O port data.
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Read XBlast GPI/O port");
        itemPtr->functionPtr= GPIORead;
        itemPtr->functionDataPtr = NULL;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    //Print LPCmodsettings struct data.
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Print settings values");
    itemPtr->functionPtr= settingsPrintData;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);
/*
    //Boot BFM BIOS.
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Boot BFM BIOS");
    itemPtr->functionPtr= DrawChildTextMenu;
    itemPtr->functionDataPtr = (void *)BFMBootMenuInit();
    TextMenuAddItem(menuPtr, itemPtr);
*/
    return menuPtr;
}
