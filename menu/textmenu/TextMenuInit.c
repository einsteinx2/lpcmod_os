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
    
    //Create the root menu - MANDATORY
    menuPtr = malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "XBlast Mod settings");
    menuPtr->firstMenuItem=NULL;

    
    if(fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_V1_TSOP) {        //No need to display this menu if no modchip is present.
        //XBlast(modchip) SETTINGS MENU
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "XBlast settings");
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

    if(fHasHardware == SYSCON_ID_V1 ||                  //No need to display this menu if no modchip is present.
       fHasHardware == SYSCON_ID_V1_TSOP ||
       fHasHardware == SYSCON_ID_XX1 ||
       fHasHardware == SYSCON_ID_XX2 ||
       fHasHardware == SYSCON_ID_XXOPX ||
       fHasHardware == SYSCON_ID_XX3 ||
       fHasHardware == SYSCON_ID_X3){                  //LCD is supported on SmartXX chips.
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
    if(fHasHardware == SYSCON_ID_V1_TSOP){
        //FLASH MENU
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Flash menu");
        itemPtr->functionPtr=DrawChildTextMenu;
        if((LPCmodSettings.OSsettings.TSOPcontrol) & 0x02)
            itemPtr->functionDataPtr = (void *)TSOPBankSelectMenuInit();
        else
            itemPtr->functionDataPtr = (void *)BankSelectInit();
        TextMenuAddItem(menuPtr, itemPtr);
    }
    else if(fHasHardware == SYSCON_ID_V1){
        //FLASH MENU
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Flash menu");
        itemPtr->functionPtr=DrawChildTextMenu;
        itemPtr->functionDataPtr = (void *)BankSelectMenuInit();
        TextMenuAddItem(menuPtr, itemPtr);
    }
    else {
        //FLASH MENU. Shown when no XBlast modchip is detected.
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Flash menu");
        itemPtr->functionPtr=DrawChildTextMenu;
        itemPtr->functionDataPtr = (void *)BankSelectInit();
        TextMenuAddItem(menuPtr, itemPtr);
    }
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
