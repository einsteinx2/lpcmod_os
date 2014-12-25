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
    int i=0;

    //No entry in this menu will have a configurable parameter.
    //Set first character to NULL to indicate no string is to be shown.
    itemPtr->szParameter[0]=0;

    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "Tools");

    if(cromwell_config==CROMWELL || fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_XT) {
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

    //Dangerous stuff is going on in there.
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Edit EEPROM content");
    itemPtr->functionPtr= warningDisplayEepromEditMenu;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    //Wipe EEPROM section that holds non-vital data.
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Reset system settings");
    itemPtr->functionPtr= wipeEEPromUserSettings;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    //Do not show this entry if 1.6/1.6b
    if(mbVersion != REV1_6){
        //128MB MEMORY TEST
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "128MB RAM test");
        itemPtr->functionPtr=showMemTest;
        itemPtr->functionDataPtr = NULL;
        TextMenuAddItem(menuPtr, itemPtr);
    }
    if((mbVersion == REV1_1 || mbVersion == REV1_0 || DEV_FEATURES) &&  //Don't show this when Xbox motherboard is not 1.0/1.1.
       (LPCmodSettings.OSsettings.TSOPcontrol)){               //Don't show if TSOP split is not enabled.
        //TSOP split manual control
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "TSOP recover force bank : ");
        if(A19controlModBoot == BNKTSOPSPLIT0)
            sprintf(itemPtr->szParameter, "%s", "Bank0");
        else if(A19controlModBoot == BNKTSOPSPLIT1)
            sprintf(itemPtr->szParameter, "%s", "Bank1");
        else
            sprintf(itemPtr->szParameter, "%s", "No");
        itemPtr->functionPtr= nextA19controlModBootValue;
        itemPtr->functionDataPtr= itemPtr->szParameter;
        itemPtr->functionLeftPtr=prevA19controlModBootValue;
        itemPtr->functionLeftDataPtr = itemPtr->szParameter;
        itemPtr->functionRightPtr=nextA19controlModBootValue;
        itemPtr->functionRightDataPtr = itemPtr->szParameter;
        TextMenuAddItem(menuPtr, itemPtr);
    }
/*
    //TSOP recovery entries. Do not show if already in TSOP recovery
    if((cromwell_config==CROMWELL || fHasHardware == SYSCON_ID_V1)
       && !TSOPRecoveryMode) {
        //TSOP recovery
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "TSOP Recovery");
        itemPtr->functionPtr=TSOPRecoveryReboot;
        itemPtr->functionDataPtr = NULL;
        TextMenuAddItem(menuPtr, itemPtr);
    }
*/
    if(DEV_FEATURES){
        //Save xblast.cfg
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Save C:\\xblast.cfg");
        itemPtr->functionPtr= saveXBlastcfg;
        itemPtr->functionDataPtr = NULL;
        TextMenuAddItem(menuPtr, itemPtr);
    }
    //Load xblast.cfg
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Load C:\\xblast.cfg");
    itemPtr->functionPtr= loadXBlastcfg;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    if(DEV_FEATURES){
        //Developers tools
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Developer tools");
        itemPtr->functionPtr= DrawChildTextMenu;
        itemPtr->functionDataPtr = (void *)DeveloperMenuInit();
        TextMenuAddItem(menuPtr, itemPtr);
    }

    return menuPtr;
}
