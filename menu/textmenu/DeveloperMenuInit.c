/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "MenuInits.h"
#include "XBlastScriptMenuActions.h"
#include "DeveloperMenuActions.h"
#include "lpcmod_v1.h"
#include "string.h"
#include "stdio.h"
#include "xblast/HardwareIdentifier.h"
#include "FatFSAccessor.h"

void DeveloperFileOpsMenuDynamic(void* drive);

TEXTMENU* DeveloperMenuInit(void)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;
    unsigned char i;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "Developer tools");

    //Write to LPC port
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Write LPC I/O");
    itemPtr->functionPtr = LPCIOWrite;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);
    
    //Read LPC port data.
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Read LPC I/O");
    itemPtr->functionPtr = LPCIORead;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    //Read SMBus data.
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Read SMBus");
    itemPtr->functionPtr = SMBusRead;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    if(fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_V1_TSOP){
        //Read GPI/O port data.
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Read XBlast GPI/O port");
        itemPtr->functionPtr = GPIORead;
        itemPtr->functionDataPtr = NULL;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    //Print LPCmodsettings struct data.
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Print BIOS Identifier");
    itemPtr->functionPtr = printBiosIdentifier;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    //Print LPCmodsettings struct data.
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Print settings values");
    itemPtr->functionPtr = settingsPrintData;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    for(i = HDD_Master; i < NbDrivesSupported; i++)
    {
        if(isFATXFormattedDrive(i))
        {
            //File operations submenu.
            itemPtr = calloc(1, sizeof(TEXTMENUITEM));
            sprintf(itemPtr->szCaption, "%s: File op", i == HDD_Master ? "Master" : "Slave");
            itemPtr->functionPtr = DeveloperFileOpsMenuDynamic;
            itemPtr->functionDataPtr = malloc(sizeof(unsigned char));
            *(unsigned char*)itemPtr->functionDataPtr = i;
            itemPtr->dataPtrAlloc = 1;
            TextMenuAddItem(menuPtr, itemPtr);
        }
    }
/*
    //Boot BFM BIOS.
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Boot BFM BIOS");
    itemPtr->functionPtr = DrawChildTextMenu;
    itemPtr->functionDataPtr = (void *)BFMBootMenuInit();
    TextMenuAddItem(menuPtr, itemPtr);
*/


    return menuPtr;
}

void DeveloperFileOpsMenuDynamic(void* drive)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;
    unsigned char nDriveIndex = *(unsigned char *)drive;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    sprintf(menuPtr->szCaption, "%s: File op", nDriveIndex == HDD_Master ? "Master" : "Slave");

    //Print LPCmodsettings struct data.
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Write To Y:");
    itemPtr->functionPtr = WriteToYDrive;
    itemPtr->functionDataPtr = drive;
    TextMenuAddItem(menuPtr, itemPtr);

    ResetDrawChildTextMenu(menuPtr);
}
