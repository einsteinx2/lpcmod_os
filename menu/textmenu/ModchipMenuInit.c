#include "MenuInits.h"
#include "lpcmod_v1.h"
#include "config.h"
#include "boot.h"
#include "ModchipMenuActions.h"
#include "string.h"
#include "lib/LPCMod/BootLPCMod.h"

TEXTMENU *ModchipMenuInit(void) {

    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;


    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "XBlast settings");

if(fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_XT){
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
    else if (LPCmodSettings.OSsettings.altBank == BNKTSOPSPLIT0)
        strcpy(itemPtr->szParameter,"TSOP bank0");            //Show this string if TSOP is split.
    else if (LPCmodSettings.OSsettings.altBank == BNKFULLTSOP)
        strcpy(itemPtr->szParameter,"TSOP");                //TSOP is not split.
    else if (LPCmodSettings.OSsettings.altBank == BNKTSOPSPLIT1)
        strcpy(itemPtr->szParameter,"TSOP bank1");
    else if (LPCmodSettings.OSsettings.altBank == BNKOS)
        strcpy(itemPtr->szParameter,"XBlast OS");
    itemPtr->functionPtr=incrementAltBank;
    itemPtr->functionDataPtr = itemPtr->szParameter;
    itemPtr->functionLeftPtr=incrementAltBank;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=incrementAltBank;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

if((fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_V1_TSOP) && 
   (mbVersion == REV1_1 || mbVersion == REV1_0 || DEV_FEATURES)){        //Don't show this when Xbox motherboard is not 1.0/1.1.

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption,"Control Xbox TSOP : ");
    sprintf(itemPtr->szParameter, "%s", (LPCmodSettings.OSsettings.TSOPcontrol)? "Yes" : "No");
    itemPtr->functionPtr= toggleTSOPcontrol;
    itemPtr->functionDataPtr= itemPtr;
    itemPtr->functionLeftPtr=toggleTSOPcontrol;
    itemPtr->functionLeftDataPtr = itemPtr;
    itemPtr->functionRightPtr=toggleTSOPcontrol;
    itemPtr->functionRightDataPtr = itemPtr;
    TextMenuAddItem(menuPtr, itemPtr);
    
}

itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption,"Hide TSOP boot icon : ");
    sprintf(itemPtr->szParameter, "%s", (LPCmodSettings.OSsettings.TSOPhide)? "Yes" : "No");
    itemPtr->functionPtr= toggleTSOPhide;
    itemPtr->functionDataPtr= itemPtr;
    itemPtr->functionLeftPtr=toggleTSOPhide;
    itemPtr->functionLeftDataPtr = itemPtr;
    itemPtr->functionRightPtr=toggleTSOPhide;
    itemPtr->functionRightDataPtr = itemPtr;
    TextMenuAddItem(menuPtr, itemPtr);

/*
 *
 * If you change order of the 4 entries above, make sure to modify functions of these entries in ModchipMenuActions.c
 *
 */

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Bank0(512KB) BIOS name");
    itemPtr->szParameter[0] = 0;
    itemPtr->functionPtr= editBIOSName;
    itemPtr->functionDataPtr= malloc(sizeof(char));
        *(char*)itemPtr->functionDataPtr = BNK512;
    itemPtr->functionDataPtrMemAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Bank1(256KB) BIOS name");
    itemPtr->functionPtr= editBIOSName;
    itemPtr->functionDataPtr= malloc(sizeof(char));
        *(char*)itemPtr->functionDataPtr = BNK256;
    itemPtr->functionDataPtrMemAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);

    if(((fHasHardware == SYSCON_ID_V1) || (fHasHardware == SYSCON_ID_V1_TSOP)) && 
       (LPCmodSettings.OSsettings.TSOPcontrol)){
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"TSOP bank0 name");
        itemPtr->functionPtr= editBIOSName;
        itemPtr->functionDataPtr= malloc(sizeof(char));
            *(char*)itemPtr->functionDataPtr = BNKTSOPSPLIT0;
        TextMenuAddItem(menuPtr, itemPtr);

        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"TSOP bank1 name");
        itemPtr->functionPtr= editBIOSName;
        itemPtr->functionDataPtr= malloc(sizeof(char));
            *(char*)itemPtr->functionDataPtr = BNKTSOPSPLIT1;
        itemPtr->functionDataPtrMemAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);
    }
    else{
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"TSOP BIOS name");
        itemPtr->functionPtr= editBIOSName;
        itemPtr->functionDataPtr= malloc(sizeof(char));
            *(char*)itemPtr->functionDataPtr = BNKFULLTSOP;
        itemPtr->functionDataPtrMemAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);
    }
    
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Reset all settings");
    itemPtr->functionPtr= resetSettings;
    itemPtr->functionDataPtr= NULL;
    TextMenuAddItem(menuPtr, itemPtr);
}
else{
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Reboot from Modchip");
    itemPtr->noSelect = NOSELECTERROR;
    TextMenuAddItem(menuPtr, itemPtr);
    
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"to edit these settings.");
    itemPtr->noSelect = NOSELECTERROR;
    TextMenuAddItem(menuPtr, itemPtr);
}

    return menuPtr;
}
