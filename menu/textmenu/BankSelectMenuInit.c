/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/

#include "MenuInits.h"
#include "lpcmod_v1.h"
#include "config.h"
#include "FlashMenuActions.h"
#include "boot.h"
#include "string.h"
#include "lib/LPCMod/BootLPCMod.h"


TEXTMENU *BankSelectMenuInit(void * bank) {
    
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;
    

    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "Select flash bank");


    //Bank0 (512KB)
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Bank0 (512KB)");
    itemPtr->functionPtr=(void *)BankSelectInit;
    itemPtr->functionDataPtr = malloc(sizeof(unsigned char));
        *(unsigned char *)itemPtr->functionDataPtr = BNK512;
    itemPtr->functionDataPtrMemAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);

    //Bank1 (256KB)
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Bank1 (256KB)");
    itemPtr->functionPtr=(void *)BankSelectInit;
    itemPtr->functionDataPtr = malloc(sizeof(unsigned char));
        *(unsigned char *)itemPtr->functionDataPtr = BNK256;
    itemPtr->functionDataPtrMemAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);

    //Bank2 (OS)
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Bank2 (OS)");
    itemPtr->functionPtr=(void *)BankSelectInit;
    itemPtr->functionDataPtr = malloc(sizeof(unsigned char));
        *(unsigned char *)itemPtr->functionDataPtr = BNKOS;
    itemPtr->functionDataPtrMemAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);
    
    return menuPtr;
}

TEXTMENU *TSOPBankSelectMenuInit(void * bank) {

    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;


    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "Select TSOP flash bank");


    //Bank0
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "TSOP bank0");
    itemPtr->functionPtr=(void *)BankSelectInit;
    itemPtr->functionDataPtr = malloc(sizeof(unsigned char));
        *(unsigned char *)itemPtr->functionDataPtr = BNKTSOPSPLIT0;
    itemPtr->functionDataPtrMemAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);

    //Bank1
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "TSOP bank1");
    itemPtr->functionPtr=(void *)BankSelectInit;
    itemPtr->functionDataPtr = malloc(sizeof(unsigned char));
        *(unsigned char *)itemPtr->functionDataPtr = BNKTSOPSPLIT1;
    itemPtr->functionDataPtrMemAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);

    return menuPtr;
}


TEXTMENU* BankSelectInit(void * bank) {
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;
    int i=0;

    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    if(TSOPRecoveryMode){
        if(*(unsigned char *)bank == BNKFULLTSOP)
           strcpy(menuPtr->szCaption, "Flash menu : Full TSOP recover");
        else if(*(unsigned char *)bank == BNKTSOPSPLIT0)
            strcpy(menuPtr->szCaption, "Flash menu : bank0 TSOP recover");
        else if(*(unsigned char *)bank == BNKTSOPSPLIT1)
            strcpy(menuPtr->szCaption, "Flash menu : bank1 TSOP recover");
        else
            strcpy(menuPtr->szCaption, "UNKNOWN BANK. GO BACK!");

        switchOSBank(*(unsigned char *)bank);

    }
    else if(fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_XT){
        if(*(unsigned char *)bank == BNKOS)
            strcpy(menuPtr->szCaption, "Flash menu : OS bank");
        else if(*(unsigned char *)bank == BNK256)
            strcpy(menuPtr->szCaption, "Flash menu : 256KB bank");
        else if(*(unsigned char *)bank == BNK512)
            strcpy(menuPtr->szCaption, "Flash menu : 512KB bank");
        else
            strcpy(menuPtr->szCaption, "UNKNOWN BANK. GO BACK!");

        switchOSBank(*(unsigned char *)bank);
    }
    else if(fHasHardware == SYSCON_ID_V1_TSOP || fHasHardware == SYSCON_ID_XT_TSOP){
        if(LPCmodSettings.OSsettings.TSOPcontrol){
    	    if(*(unsigned char *)bank == BNKTSOPSPLIT0)
                strcpy(menuPtr->szCaption, "Flash menu : TSOP bank0");
            else if(*(unsigned char *)bank == BNKTSOPSPLIT1)
                strcpy(menuPtr->szCaption, "Flash menu : TSOP bank1");
            else
                strcpy(menuPtr->szCaption, "UNKNOWN BANK. GO BACK!");
            switchOSBank(*(unsigned char *)bank);
        }
        else{
            strcpy(menuPtr->szCaption, "Flash menu : TSOP");
            switchOSBank(BNKFULLTSOP);	//Release everything.
        }
    }
    else {
        strcpy(menuPtr->szCaption, "Flash menu : Unknown device");
    }
    
#ifdef LWIP
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
       sprintf(itemPtr->szCaption,"Net Flash");
    itemPtr->functionPtr= enableNetflash;
    itemPtr->functionDataPtr= malloc(sizeof(unsigned char));
        *(unsigned char *)itemPtr->functionDataPtr= BIOS_NETFLASH;
    TextMenuAddItem(menuPtr, itemPtr);
#if DEV_FEATURES
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
       sprintf(itemPtr->szCaption,"Web Update");
    itemPtr->functionPtr= enableWebupdate;
    itemPtr->functionDataPtr= NULL;
    TextMenuAddItem(menuPtr, itemPtr);
#endif
#endif

    for (i=0; i<2; ++i) {
        if (tsaHarddiskInfo[i].m_fDriveExists && tsaHarddiskInfo[i].m_fAtapi) {
             char *driveName=malloc(sizeof(char)*32);
            itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
            memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
            sprintf(itemPtr->szCaption,"CD Flash (image.bin)");// (hd%c)",i ? 'b':'a');
            itemPtr->functionPtr= FlashBiosFromCD;
            itemPtr->functionDataPtr = malloc(sizeof(int));
            *(int*)itemPtr->functionDataPtr = i;
            itemPtr->functionDataPtrMemAlloc = true;    //Could pass &i... meh.
            TextMenuAddItem(menuPtr, itemPtr);
        }
    }

    //Only Master HDD will be supported here.
    if (tsaHarddiskInfo[0].m_fDriveExists && !tsaHarddiskInfo[0].m_fAtapi){
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
          sprintf(itemPtr->szCaption,"HDD Flash");
        itemPtr->functionPtr=DrawChildTextMenu;
        itemPtr->functionDataPtr = (void *)HDDFlashMenuInit();
        TextMenuAddItem(menuPtr, itemPtr);
    }

/*
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Selected bank");
    itemPtr->functionPtr=DrawChildTextMenu;
    itemPtr->functionDataPtr = (void *)FlashMenuInit();
    TextMenuAddItem(menuPtr, itemPtr);*/
    if(fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_XT ||
       (fHasHardware == SYSCON_ID_V1_TSOP && (LPCmodSettings.OSsettings.TSOPcontrol))){
        ResetDrawChildTextMenu(menuPtr);
    }
    return menuPtr;
}

