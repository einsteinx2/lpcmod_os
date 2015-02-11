/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "boot.h"
#include "BootIde.h"
#include "TextMenu.h"
#include "HDDMenuActions.h"

TEXTMENU *HDDOperationsMenuInit(void * drive);
TEXTMENU *LargeHDDMenuInit(void * drive);
TEXTMENU *HDDSMARTOperationsMenuInit(void * drive);
TEXTMENU *HDDFormatMenuInit(void * drive);
TEXTMENU *HDDLockUnlockMenuInit(void * drive);

TEXTMENU *HDDMenuInit(void) {
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;
    int i=0;

    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "HDD Menu");

    for (i=0; i<2; ++i) {
        if (tsaHarddiskInfo[i].m_fDriveExists && !tsaHarddiskInfo[i].m_fAtapi) {
            //If it's not ATAPI, it must be IDE
            //Add menu entry for corresponding HDD
            itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
            memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
            sprintf(itemPtr->szCaption, "%s",i ? "Slave":"Master");
            sprintf(itemPtr->szParameter, "%s"," HDD");
            itemPtr->functionPtr= (void *)HDDOperationsMenuInit;
            itemPtr->functionDataPtr = malloc(sizeof(u8));
                *(u8*)itemPtr->functionDataPtr = i;
            itemPtr->functionDataPtrMemAlloc = true;
            TextMenuAddItem(menuPtr, itemPtr);
        }
    }

    return menuPtr;
}


TEXTMENU *HDDOperationsMenuInit(void * drive){
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;
    u8 nDriveIndex = 1;

    nDriveIndex = *(u8 *) drive;

    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    sprintf(menuPtr->szCaption, "%s HDD", nDriveIndex ? "Slave":"Master");


    if((tsaHarddiskInfo[nDriveIndex].m_securitySettings &0x0001)==0x0001) {       //Drive Security feature supported.
        //HDD Lock/Unlock menu
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"Lock/Unlock menu");
        itemPtr->functionPtr= (void *)HDDLockUnlockMenuInit;
        itemPtr->functionDataPtr = malloc(sizeof(u8));
            *(u8*)itemPtr->functionDataPtr = nDriveIndex;
        itemPtr->functionDataPtrMemAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    //Add a 'display HDD info' menu
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Display HDD info");
    itemPtr->functionPtr= DisplayHDDInfo;
    itemPtr->functionDataPtr = malloc(sizeof(u8));
        *(u8*)itemPtr->functionDataPtr = nDriveIndex;
    itemPtr->functionDataPtrMemAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);

    if(DEV_FEATURES){
        if(tsaHarddiskInfo[nDriveIndex].m_fHasSMARTcapabilities){
            //S.M.A.R.T. menu
            itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
            memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
            sprintf(itemPtr->szCaption,"S.M.A.R.T. menu");
            itemPtr->functionPtr= (void *)HDDSMARTOperationsMenuInit;
            itemPtr->functionDataPtr = malloc(sizeof(u8));
                *(u8*)itemPtr->functionDataPtr = nDriveIndex;
            itemPtr->functionDataPtrMemAlloc = true;
            TextMenuAddItem(menuPtr, itemPtr);
        }
    }

    //Format menu
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Partition format menu");
    itemPtr->functionPtr= (void *)HDDFormatMenuInit;
    itemPtr->functionDataPtr = malloc(sizeof(u8));
        *(u8*)itemPtr->functionDataPtr = nDriveIndex;
    itemPtr->functionDataPtrMemAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);



    ResetDrawChildTextMenu(menuPtr);

    return menuPtr;
}


TEXTMENU *LargeHDDMenuInit(void * drive) {
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;
    u8 nDriveIndex = 1;
    
    nDriveIndex = *(u8 *) drive;

    //Amount of free sectors after standard partitions
    unsigned long nExtendSectors = tsaHarddiskInfo[nDriveIndex].m_dwCountSectorsTotal - SECTOR_EXTEND;

    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    sprintf(menuPtr->szCaption, "Large HDD format options : %s", nDriveIndex ? "Slave":"Master");

    //If lbacount >= minimum amount per partition.
    if(nExtendSectors > (SECTORS_SYSTEM + SECTORS_SYSTEM)){
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"F:, G: Split evenly");
        itemPtr->functionPtr= FormatDriveFG;
        itemPtr->functionDataPtr = malloc(sizeof(u8));
            *(u8 *)itemPtr->functionDataPtr = nDriveIndex | F_GEQUAL;
        itemPtr->functionDataPtrMemAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    //If lbacount is high enough to max out a F: partition and still have enough left to create a G partition
    if(nExtendSectors > (LBASIZE_1024GB + SECTORS_SYSTEM)){
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"Max F:, G: takes the rest");
        itemPtr->functionPtr= FormatDriveFG;
        itemPtr->functionDataPtr = malloc(sizeof(u8));
            *(u8 *)itemPtr->functionDataPtr = nDriveIndex | FMAX_G;
        itemPtr->functionDataPtrMemAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    //if lbacount is high enough to create G: partition but not too high to waste space because G: would be maxed out.
    if((nExtendSectors > (LBASIZE_137GB + SECTORS_SYSTEM)) && ((nExtendSectors - LBASIZE_137GB) < LBASIZE_1024GB)){
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"F: = 120GB, G: takes the rest");
        itemPtr->functionPtr= FormatDriveFG;
        itemPtr->functionDataPtr = malloc(sizeof(u8));
            *(u8 *)itemPtr->functionDataPtr = nDriveIndex | F137_G;
        itemPtr->functionDataPtrMemAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    //If lbacount is not too high to waste space because F: can't get bigger.
    if(nExtendSectors < LBASIZE_1024GB){
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"F: take all, no G:");
        itemPtr->functionPtr= FormatDriveFG;
        itemPtr->functionDataPtr = malloc(sizeof(u8));
            *(u8 *)itemPtr->functionDataPtr = nDriveIndex | F_NOG;
        itemPtr->functionDataPtrMemAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    ResetDrawChildTextMenu(menuPtr);

    return menuPtr;
}

TEXTMENU *HDDSMARTOperationsMenuInit(void * drive) {
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;
    u8 nDriveIndex = 1;

    nDriveIndex = *(u8 *) drive;

    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    sprintf(menuPtr->szCaption, "S.M.A.R.T. menu : %s", nDriveIndex ? "Slave":"Master");

    //SMART Enable/Disable
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    if(tsaHarddiskInfo[nDriveIndex].m_fSMARTEnabled) {
        sprintf(itemPtr->szCaption,"Disable");
    }
    else {
        sprintf(itemPtr->szCaption,"Enable");
    }
    sprintf(itemPtr->szParameter, " S.M.A.R.T.");
    itemPtr->szParameter[50] = nDriveIndex;
    itemPtr->functionPtr= AssertSMARTEnableDisable;
    itemPtr->functionDataPtr = itemPtr;
    TextMenuAddItem(menuPtr, itemPtr);

    //SMART Enable/Disable
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Read S.M.A.R.T. status");
    itemPtr->functionPtr= CheckSMARTRETURNSTATUS;
    itemPtr->functionDataPtr = malloc(sizeof(u8));
        *(u8 *)itemPtr->functionDataPtr = nDriveIndex;
    itemPtr->functionDataPtrMemAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);

    ResetDrawChildTextMenu(menuPtr);

    return menuPtr;
}


TEXTMENU *HDDFormatMenuInit(void * drive) {
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;
    u8 nDriveIndex = 1;

    nDriveIndex = *(u8 *) drive;

    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    sprintf(menuPtr->szCaption, "Partition format menu : %s", nDriveIndex ? "Slave":"Master");

    if(tsaHarddiskInfo[nDriveIndex].m_fHasMbr != -1){     //MBR contains standard basic partition entries.
        //FORMAT C: drive
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"Format C drive");
        itemPtr->functionPtr= FormatDriveC;
        itemPtr->functionDataPtr = malloc(sizeof(u8));
            *(u8*)itemPtr->functionDataPtr = nDriveIndex;
        itemPtr->functionDataPtrMemAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);

        //FORMAT E: drive
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"Format E drive");
        itemPtr->functionPtr= FormatDriveE;
        itemPtr->functionDataPtr = malloc(sizeof(u8));
            *(u8*)itemPtr->functionDataPtr = nDriveIndex;
        itemPtr->functionDataPtrMemAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);

        //FORMAT X:, Y: and Z: drives.
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"Format cache drives");
        itemPtr->functionPtr= FormatCacheDrives;
        itemPtr->functionDataPtr = malloc(sizeof(u8));
            *(u8*)itemPtr->functionDataPtr = nDriveIndex;
        itemPtr->functionDataPtrMemAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);

        //If there's enough sectors to make F and/or G drive(s).
        if(tsaHarddiskInfo[nDriveIndex].m_dwCountSectorsTotal >= (SECTOR_EXTEND + SECTORS_SYSTEM)){
            //Format Larger drives option menu.
            itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
            memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
            sprintf(itemPtr->szCaption,"Large HDD format");
            itemPtr->functionPtr= (void *)LargeHDDMenuInit;
            itemPtr->functionDataPtr = malloc(sizeof(u8));
                *(u8 *)itemPtr->functionDataPtr = nDriveIndex;
            itemPtr->functionDataPtrMemAlloc = true;
            TextMenuAddItem(menuPtr, itemPtr);
        }
    }
    else{
        //Print message.
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"Unsupported partition scheme...");
        itemPtr->functionPtr= NULL;
        itemPtr->functionDataPtr = NULL;
        itemPtr->noSelect = NOSELECTERROR;
        TextMenuAddItem(menuPtr, itemPtr);

        //Print message.
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"XBlast OS will not format this HDD!");
        itemPtr->functionPtr= NULL;
        itemPtr->functionDataPtr = NULL;
        itemPtr->noSelect = NOSELECTERROR;
        TextMenuAddItem(menuPtr, itemPtr);
    }
    ResetDrawChildTextMenu(menuPtr);

    return menuPtr;
}

TEXTMENU *HDDLockUnlockMenuInit(void * drive) {
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;
    u8 nDriveIndex = 1;

    nDriveIndex = *(u8 *) drive;

    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    sprintf(menuPtr->szCaption, "Lock/Unlock menu : %s", nDriveIndex ? "Slave":"Master");


    //This drive is locked - produce an unlock menu
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    if((tsaHarddiskInfo[nDriveIndex].m_securitySettings &0x0002)==0x0002) {
        sprintf(itemPtr->szCaption,"Unlock HDD");
    }
    else {
        sprintf(itemPtr->szCaption,"Lock HDD");
    }
    itemPtr->szParameter[50] = nDriveIndex;
    itemPtr->functionPtr= AssertLockUnlock;
    itemPtr->functionDataPtr = itemPtr;
    TextMenuAddItem(menuPtr, itemPtr);


    //Add a 'display password' menu
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Display HDD password");
    itemPtr->functionPtr= DisplayHDDPassword;
    itemPtr->functionDataPtr = malloc(sizeof(u8));
        *(u8*)itemPtr->functionDataPtr = nDriveIndex;
    itemPtr->functionDataPtrMemAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    if((tsaHarddiskInfo[nDriveIndex].m_securitySettings &0x0002)==0x0002) {
        sprintf(itemPtr->szCaption,"Unlock");
    }
    else {
        sprintf(itemPtr->szCaption,"Lock");
    }
    sprintf(itemPtr->szParameter, " from network");
    itemPtr->szParameter[50] = nDriveIndex;
    itemPtr->functionPtr= AssertLockUnlockFromNetwork;
    itemPtr->functionDataPtr = itemPtr;
    TextMenuAddItem(menuPtr, itemPtr);

    ResetDrawChildTextMenu(menuPtr);

    return menuPtr;
}
