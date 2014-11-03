#include "lpcmod_v1.h"
#include "include/config.h"
#include "TextMenu.h"
#include "include/boot.h"
#include "ModchipMenuActions.h"

TEXTMENU *ModchipMenuInit(void) {

    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;


    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "XBlast settings");

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption,"Idle timeout : ");
    sprintf(itemPtr->szParameter, "%ds", LPCmodSettings.OSsettings.bootTimeout);
    itemPtr->functionPtr= NULL;
    itemPtr->functionDataPtr= NULL;
    itemPtr->functionLeftPtr=decrementbootTimeout;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=incrementbootTimeout;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    itemPtr->functionLTPtr=decrementbootTimeout;
    itemPtr->functionLTDataPtr = itemPtr->szParameter;
    itemPtr->functionRTPtr=incrementbootTimeout;
    itemPtr->functionRTDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption,"Quickboot : ");
    sprintf(itemPtr->szParameter, "%s", LPCmodSettings.OSsettings.Quickboot? "Yes" : "No");
    itemPtr->functionPtr= toggleQuickboot;
    itemPtr->functionDataPtr= itemPtr->szParameter;
    itemPtr->functionLeftPtr=toggleQuickboot;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=toggleQuickboot;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

/*
 * The 3 following menu entries must be in line.
 *
 *
 */
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption,"Power button boot : ");
    if(LPCmodSettings.OSsettings.activeBank == BNK512)
        strcpy(itemPtr->szParameter,"512KB");
    else if (LPCmodSettings.OSsettings.activeBank == BNK256)
        strcpy(itemPtr->szParameter,"256KB");
    else if (LPCmodSettings.OSsettings.activeBank == BNKTSOPSPLIT0)
        strcpy(itemPtr->szParameter,"TSOP bank0");            //Show this string if TSOP is split.
    else if (LPCmodSettings.OSsettings.activeBank == BNKFULLTSOP)
        strcpy(itemPtr->szParameter,"TSOP");                //TSOP is not split.
    else if (LPCmodSettings.OSsettings.activeBank == BNKTSOPSPLIT1)
        strcpy(itemPtr->szParameter,"TSOP bank1");
    itemPtr->functionPtr=incrementActiveBank;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr=decrementActiveBank;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=incrementActiveBank;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);
    
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption,"Eject button boot : ");
    if(LPCmodSettings.OSsettings.altBank == BNK512)
        strcpy(itemPtr->szParameter,"512KB");
    else if (LPCmodSettings.OSsettings.altBank == BNK256)
        strcpy(itemPtr->szParameter,"256KB");
    else if (LPCmodSettings.OSsettings.activeBank == BNKTSOPSPLIT0)
        strcpy(itemPtr->szParameter,"TSOP bank0");            //Show this string if TSOP is split.
    else if (LPCmodSettings.OSsettings.activeBank == BNKFULLTSOP)
        strcpy(itemPtr->szParameter,"TSOP");                //TSOP is not split.
    else if (LPCmodSettings.OSsettings.activeBank == BNKTSOPSPLIT1)
        strcpy(itemPtr->szParameter,"TSOP bank1");
    itemPtr->functionPtr=incrementAltBank;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr=incrementAltBank;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=incrementAltBank;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

if(mbVersion == REV1_1 || mbVersion == REV1_0){        //Don't show this when Xbox motherboard is not 1.0/1.1.

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption,"Control Xbox TSOP : ");
    sprintf(itemPtr->szParameter, "%s", (LPCmodSettings.OSsettings.TSOPcontrol) & 0x02? "Yes" : "No");
    itemPtr->functionPtr= toggleTSOPControl;
    itemPtr->functionDataPtr= itemPtr;
    itemPtr->functionLeftPtr=toggleTSOPControl;
    itemPtr->functionLeftDataPtr = itemPtr;
    itemPtr->functionRightPtr=toggleTSOPControl;
    itemPtr->functionRightDataPtr = itemPtr;
    TextMenuAddItem(menuPtr, itemPtr);
    
}

/*
 *
 * If you change order of the 3 entries above, make sure to modify functions of these entries in ModchipMenuActions.c
 *
 */

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Bank0(512KB) BIOS name");
    itemPtr->szParameter[0] = 0;
    itemPtr->functionPtr= editBIOSName;
    itemPtr->functionDataPtr= malloc(sizeof(char));
        *(char*)itemPtr->functionDataPtr = BNK512;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Bank1(256KB) BIOS name");
    itemPtr->functionPtr= editBIOSName;
    itemPtr->functionDataPtr= malloc(sizeof(char));
        *(char*)itemPtr->functionDataPtr = BNK256;
    TextMenuAddItem(menuPtr, itemPtr);

    if(((fHasHardware == SYSCON_ID_V1) || (fHasHardware == SYSCON_ID_V1_TSOP)) && 
       (LPCmodSettings.OSsettings.TSOPcontrol & 0x02)){
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"TSOP bank0 name");
        itemPtr->functionDataPtr= malloc(sizeof(char));
                *(char*)itemPtr->functionDataPtr = BNKTSOPSPLIT0;
        TextMenuAddItem(menuPtr, itemPtr);

        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"TSOP bank1 name");
        itemPtr->functionDataPtr= malloc(sizeof(char));
                *(char*)itemPtr->functionDataPtr = BNKTSOPSPLIT1;
        TextMenuAddItem(menuPtr, itemPtr);
    }
    else{
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"TSOP BIOS name");
        itemPtr->functionPtr= editBIOSName;
        itemPtr->functionDataPtr= malloc(sizeof(char));
            *(char*)itemPtr->functionDataPtr = BNKFULLTSOP;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Reset all settings");
    itemPtr->functionPtr= resetSettings;
    itemPtr->functionDataPtr= NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    return menuPtr;
}
