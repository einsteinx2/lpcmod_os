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
#include "xblast/HardwareIdentifier.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "BootIde.h"
#include "HttpServer.h"


TEXTMENU* BankSelectMenuInit(void)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "Select flash bank");

    //Bank0 (512KB)
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Bank0 (512KB)");
    itemPtr->functionPtr = BankSelectDynamic;
    itemPtr->functionDataPtr = malloc(sizeof(FlashBank));
    *(FlashBank *)itemPtr->functionDataPtr = FlashBank_512Bank;
    itemPtr->dataPtrAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);

    //Bank1 (256KB)
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Bank1 (256KB)");
    itemPtr->functionPtr = BankSelectDynamic;
    itemPtr->functionDataPtr = malloc(sizeof(FlashBank));
    *(FlashBank *)itemPtr->functionDataPtr = FlashBank_256Bank;
    itemPtr->dataPtrAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);

    //Bank2 (OS)
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Bank2 (OS)");
    itemPtr->functionPtr = BankSelectDynamic;
    itemPtr->functionDataPtr = malloc(sizeof(FlashBank));
    *(FlashBank *)itemPtr->functionDataPtr = FlashBank_OSBank;
    itemPtr->dataPtrAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);
    
    return menuPtr;
}

void TSOPBankSelectMenuDynamic(void* bank)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "Select TSOP flash bank");

    //Bank0
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "TSOP bank0");
    itemPtr->functionPtr = BankSelectDynamic;
    itemPtr->functionDataPtr = malloc(sizeof(FlashBank));
    *(FlashBank *)itemPtr->functionDataPtr = FlashBank_SplitTSOP0Bank;
    itemPtr->dataPtrAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);

    //Bank1
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "TSOP bank1");
    itemPtr->functionPtr = BankSelectDynamic;
    itemPtr->functionDataPtr = malloc(sizeof(FlashBank));
    *(FlashBank *)itemPtr->functionDataPtr = FlashBank_SplitTSOP1Bank;
    itemPtr->dataPtrAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);

    ResetDrawChildTextMenu(menuPtr);
}

void BankSelectDynamic(void* bank)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;
    int i = 0;
    FlashBank target = *(FlashBank *)bank;

    XBlastLogger(DEBUG_GENERAL_UI, DBG_LVL_TRACE, "Generating menu.");
    menuPtr = calloc(1, sizeof(TEXTMENU));

    if(TSOPRecoveryMode)
    {
        if(target == FlashBank_FullTSOPBank)
        {
           strcpy(menuPtr->szCaption, "Flash menu : Full TSOP recover");
        }
        else if(target == FlashBank_SplitTSOP0Bank)
        {
            strcpy(menuPtr->szCaption, "Flash menu : bank0 TSOP recover");
        }
        else if(target == FlashBank_SplitTSOP1Bank)
        {
            strcpy(menuPtr->szCaption, "Flash menu : bank1 TSOP recover");
        }
        else
        {
            strcpy(menuPtr->szCaption, "UNKNOWN BANK. GO BACK!");
            target = FlashBank_NoBank;
        }

        switchOSBank(target);

    }
    else if(isXBlastOnLPC())
    {
        if(target == FlashBank_OSBank)
        {
            strcpy(menuPtr->szCaption, "Flash menu : OS bank");
        }
        else if(target == FlashBank_256Bank)
        {
            strcpy(menuPtr->szCaption, "Flash menu : 256KB bank");
        }
        else if(target == FlashBank_512Bank)
        {
            strcpy(menuPtr->szCaption, "Flash menu : 512KB bank");
        }
        else
        {
            strcpy(menuPtr->szCaption, "UNKNOWN BANK. GO BACK!");
            target = FlashBank_NoBank;
        }

        switchOSBank(target);
    }
    else if(isXBlastOnTSOP())
    {
        if(LPCmodSettings.OSsettings.TSOPcontrol)
        {
    	    if(target == FlashBank_SplitTSOP0Bank)
    	    {
                strcpy(menuPtr->szCaption, "Flash menu : TSOP bank0");
    	    }
            else if(target == FlashBank_SplitTSOP1Bank)
            {
                strcpy(menuPtr->szCaption, "Flash menu : TSOP bank1");
            }
            else
            {
                strcpy(menuPtr->szCaption, "UNKNOWN BANK. GO BACK!");
                target = FlashBank_NoBank;
            }

            switchOSBank(target);
        }
        else
        {
            strcpy(menuPtr->szCaption, "Flash menu : TSOP");
            switchOSBank(FlashBank_FullTSOPBank);	//Release everything.
        }
    }
    else
    {
        strcpy(menuPtr->szCaption, "Flash menu : Unknown device");
        target = FlashBank_NoBank;
    }
    
#ifdef LWIP
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Net Flash");
    itemPtr->functionPtr = enableNetflash;
    itemPtr->functionDataPtr = malloc(sizeof(WebServerOps));
    *(WebServerOps *)itemPtr->functionDataPtr = WebServerOps_BIOSFlash;
    itemPtr->dataPtrAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);
#ifdef DEV_FEATURES
    itemPtr = malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Web Update");
    itemPtr->functionPtr = enableWebupdate;
    itemPtr->functionDataPtr= NULL;
    TextMenuAddItem(menuPtr, itemPtr);
#endif
#endif

    for (i = 0; i < 2; ++i)
    {
        if(BootIdeDeviceConnected(i) && BootIdeDeviceIsATAPI(i))
        {
            itemPtr = calloc(1, sizeof(TEXTMENUITEM));
            strcpy(itemPtr->szCaption, "CD Flash (image.bin)");// (hd%c)",i ? 'b':'a');
            itemPtr->functionPtr = FlashBiosFromCD;
            itemPtr->functionDataPtr = malloc(sizeof(int));
            *(int*)itemPtr->functionDataPtr = i;
            itemPtr->dataPtrAlloc = true;
            TextMenuAddItem(menuPtr, itemPtr);
        }
    }

    //Only Master HDD will be supported here.
    if(BootIdeDeviceConnected(0) && 0 == BootIdeDeviceIsATAPI(0))
    {
        XBlastLogger(DEBUG_GENERAL_UI, DBG_LVL_DEBUG, "Generating menu for HDD%u", 0);
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "HDD Flash");
        itemPtr->functionPtr = HDDFlashMenuDynamic;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    ResetDrawChildTextMenu(menuPtr);
    switchOSBank(FlashBank_OSBank);
}

