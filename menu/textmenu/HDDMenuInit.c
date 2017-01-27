/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "MenuInits.h"
#include "boot.h"
#include "BootIde.h"
#include "HDDMenuActions.h"
#include "string.h"

void HDDOperationsMenuDynamic(void * drive);
void HDDSMARTOperationsMenuDynamic(void * drive);
void HDDFormatMenuDynamic(void * drive);
void HDDLockUnlockMenuDynamic(void * drive);

TEXTMENU* HDDMenuInit(void)
{
    TEXTMENUITEM* itemPtr = NULL;
    TEXTMENU* menuPtr;
    int i = 0;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "HDD Menu");

    for(i = 0; i < 2; ++i)
    {
        if(tsaHarddiskInfo[i].m_fDriveExists && tsaHarddiskInfo[i].m_fAtapi == 0)
        {
            //If it's not ATAPI, it must be IDE
            //Add menu entry for corresponding HDD
            itemPtr = calloc(1, sizeof(TEXTMENUITEM));
            sprintf(itemPtr->szCaption, "%s",i ? "Slave":"Master");
            sprintf(itemPtr->szParameter, "%s"," HDD");
            itemPtr->functionPtr = HDDOperationsMenuDynamic;
            itemPtr->functionDataPtr = malloc(sizeof(unsigned char));
            *(unsigned char *)itemPtr->functionDataPtr = i;
            itemPtr->dataPtrAlloc = true;
            TextMenuAddItem(menuPtr, itemPtr);
        }
    }

    if(itemPtr == NULL)
    {
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption, "No Hard Drive");
        itemPtr->noSelect = NOSELECTERROR;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    return menuPtr;
}


void HDDOperationsMenuDynamic(void* drive){
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;
    unsigned char* nDriveIndex = malloc(sizeof(unsigned char));
    *nDriveIndex = *(unsigned char *)drive;

    menuPtr = calloc(1, sizeof(TEXTMENU));

    sprintf(menuPtr->szCaption, "%s HDD", *nDriveIndex ? "Slave":"Master");

    if((tsaHarddiskInfo[*nDriveIndex].m_securitySettings &0x0001)==0x0001)        //Drive Security feature supported.
    {
        //HDD Lock/Unlock menu
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"Lock/Unlock menu");
        itemPtr->functionPtr = HDDLockUnlockMenuDynamic;
        itemPtr->functionDataPtr = nDriveIndex;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    //Add a 'display HDD info' menu
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Display HDD info");
    itemPtr->functionPtr = DisplayHDDInfo;
    itemPtr->functionDataPtr = nDriveIndex;
    TextMenuAddItem(menuPtr, itemPtr);

    if(tsaHarddiskInfo[*nDriveIndex].m_fHasSMARTcapabilities){
        //S.M.A.R.T. menu
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"S.M.A.R.T. menu");
        itemPtr->functionPtr = HDDSMARTOperationsMenuDynamic;
        itemPtr->functionDataPtr = nDriveIndex;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    //Format menu
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Partition format menu");
    itemPtr->functionPtr = HDDFormatMenuDynamic;
    itemPtr->functionDataPtr = nDriveIndex;
    itemPtr->dataPtrAlloc = true;   //Signal only once as allocated mem is shared on all entries.
    TextMenuAddItem(menuPtr, itemPtr);

    ResetDrawChildTextMenu(menuPtr);
}


void LargeHDDMenuDynamic(void* drive)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;
    unsigned char nDriveIndex = *(unsigned char *)drive;
    
    //Amount of free sectors after standard partitions
    unsigned long nExtendSectors = tsaHarddiskInfo[nDriveIndex].m_dwCountSectorsTotal - SECTOR_EXTEND;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    sprintf(menuPtr->szCaption, "Large HDD format options : %s", nDriveIndex ? "Slave":"Master");

    //If lbacount >= minimum amount per partition.
    if(nExtendSectors > (SECTORS_SYSTEM + SECTORS_SYSTEM))
    {
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"F:, G: Split evenly");
        itemPtr->functionPtr = FormatDriveFG;
        itemPtr->functionDataPtr = malloc(sizeof(unsigned char));
        *(unsigned char *)itemPtr->functionDataPtr = nDriveIndex | F_GEQUAL;
        itemPtr->dataPtrAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    //If lbacount is high enough to max out a F: partition and still have enough left to create a G partition
    if(nExtendSectors > (LBASIZE_1024GB + SECTORS_SYSTEM))
    {
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"Max F:, G: takes the rest");
        itemPtr->functionPtr = FormatDriveFG;
        itemPtr->functionDataPtr = malloc(sizeof(unsigned char));
        *(unsigned char *)itemPtr->functionDataPtr = nDriveIndex | FMAX_G;
        itemPtr->dataPtrAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    //if lbacount is high enough to create G: partition but not too high to waste space because G: would be maxed out.
    if((nExtendSectors > (LBASIZE_137GB + SECTORS_SYSTEM)) && ((nExtendSectors - LBASIZE_137GB) < LBASIZE_1024GB))
    {
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"F: = 120GB, G: takes the rest");
        itemPtr->functionPtr = FormatDriveFG;
        itemPtr->functionDataPtr = malloc(sizeof(unsigned char));
        *(unsigned char *)itemPtr->functionDataPtr = nDriveIndex | F137_G;
        itemPtr->dataPtrAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    //If lbacount is not too high to waste space because F: can't get bigger.
    if(nExtendSectors < LBASIZE_1024GB)
    {
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"F: take all, no G:");
        itemPtr->functionPtr = FormatDriveFG;
        itemPtr->functionDataPtr = malloc(sizeof(unsigned char));
        *(unsigned char *)itemPtr->functionDataPtr = nDriveIndex | F_NOG;
        itemPtr->dataPtrAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    ResetDrawChildTextMenu(menuPtr);
}

void HDDSMARTOperationsMenuDynamic(void* drive)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;
    unsigned char nDriveIndex = *(unsigned char *) drive;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    sprintf(menuPtr->szCaption, "S.M.A.R.T. menu : %s", nDriveIndex ? "Slave":"Master");

    //SMART Enable/Disable
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    if(tsaHarddiskInfo[nDriveIndex].m_fSMARTEnabled)
    {
        sprintf(itemPtr->szCaption,"Disable");
    }
    else
    {
        sprintf(itemPtr->szCaption,"Enable");
    }
    sprintf(itemPtr->szParameter, " S.M.A.R.T.");
    itemPtr->functionPtr = AssertSMARTEnableDisable;
    LockUnlockCommonParams* customStruct = malloc(sizeof(LockUnlockCommonParams));
    customStruct->driveIndex = nDriveIndex;
    customStruct->string1 = itemPtr->szCaption;
    itemPtr->functionDataPtr = customStruct;
    TextMenuAddItem(menuPtr, itemPtr);

    //SMART Enable/Disable
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Read S.M.A.R.T. status");
    itemPtr->functionPtr = CheckSMARTRETURNSTATUS;
    itemPtr->functionDataPtr = customStruct;
    itemPtr->dataPtrAlloc = true;   //Signal only one since it's the same struct for 2 entries.
    TextMenuAddItem(menuPtr, itemPtr);

    ResetDrawChildTextMenu(menuPtr);
}


void HDDFormatMenuDynamic(void* drive)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;
    unsigned char* nDriveIndex = malloc(sizeof(unsigned char));
    *nDriveIndex = *(unsigned char *)drive;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    sprintf(menuPtr->szCaption, "Partition format menu : %s", *nDriveIndex ? "Slave":"Master");

    if(tsaHarddiskInfo[*nDriveIndex].m_fHasMbr != -1)     //MBR contains standard basic partition entries.
    {
        //FORMAT C: drive
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"Format C drive");
        itemPtr->functionPtr = FormatDriveC;
        itemPtr->functionDataPtr = nDriveIndex;
        itemPtr->dataPtrAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);

        //FORMAT E: drive
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"Format E drive");
        itemPtr->functionPtr = FormatDriveE;
        itemPtr->functionDataPtr = nDriveIndex;
        itemPtr->dataPtrAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);

        //FORMAT X:, Y: and Z: drives.
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"Format cache drives");
        itemPtr->functionPtr = FormatCacheDrives;
        itemPtr->functionDataPtr = nDriveIndex;
        itemPtr->dataPtrAlloc = true;
        TextMenuAddItem(menuPtr, itemPtr);

        //If there's enough sectors to make F and/or G drive(s).
        if(tsaHarddiskInfo[*nDriveIndex].m_dwCountSectorsTotal >= (SECTOR_EXTEND + SECTORS_SYSTEM))
        {
            //Format Larger drives option menu.
            itemPtr = calloc(1, sizeof(TEXTMENUITEM));
            sprintf(itemPtr->szCaption,"Large HDD format");
            itemPtr->functionPtr = LargeHDDMenuDynamic;
            itemPtr->functionDataPtr = nDriveIndex;
            itemPtr->dataPtrAlloc = true;
            TextMenuAddItem(menuPtr, itemPtr);
        }
    }
    else
    {
        //Print message.
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"Unsupported partition scheme...");
        itemPtr->functionPtr = NULL;
        itemPtr->functionDataPtr = NULL;
        itemPtr->noSelect = NOSELECTERROR;
        TextMenuAddItem(menuPtr, itemPtr);

        //Print message.
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"XBlast OS will not format this HDD!");
        itemPtr->functionPtr = NULL;
        itemPtr->functionDataPtr = NULL;
        itemPtr->noSelect = NOSELECTERROR;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    ResetDrawChildTextMenu(menuPtr);
}

void HDDLockUnlockMenuDynamic(void* drive)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;
    LockUnlockCommonParams* inputParam = malloc(sizeof(LockUnlockCommonParams));
    inputParam->driveIndex = *(unsigned char *) drive;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    sprintf(menuPtr->szCaption, "Lock/Unlock menu : %s", inputParam->driveIndex ? "Slave":"Master");

    //This drive is locked - produce an unlock menu

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    if((tsaHarddiskInfo[inputParam->driveIndex].m_securitySettings & 0x0002) == 0x0002)
    {
        sprintf(itemPtr->szCaption,"Unl");
    }
    else
    {
        sprintf(itemPtr->szCaption,"L");
    }
    inputParam->string1 = itemPtr->szCaption;

    sprintf(itemPtr->szParameter, "ock HDD");
    itemPtr->functionPtr = AssertLockUnlock;
    itemPtr->functionDataPtr = inputParam;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    if((tsaHarddiskInfo[inputParam->driveIndex].m_securitySettings & 0x0002) == 0x0002)
    {
        sprintf(itemPtr->szCaption,"Unl");
    }
    else
    {
        sprintf(itemPtr->szCaption,"L");
    }
    inputParam->string2 = itemPtr->szCaption;

    sprintf(itemPtr->szParameter, "ock HDD from network");
    itemPtr->functionPtr = AssertLockUnlockFromNetwork;
    itemPtr->functionDataPtr = inputParam;
    TextMenuAddItem(menuPtr, itemPtr);


    //Add a 'display password' menu
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    sprintf(itemPtr->szCaption,"Display HDD password");
    itemPtr->functionPtr = DisplayHDDPassword;
    itemPtr->functionDataPtr = inputParam;
    itemPtr->dataPtrAlloc = true;   //Signal only once as allocated mem is shared on all entries.
    TextMenuAddItem(menuPtr, itemPtr);

    ResetDrawChildTextMenu(menuPtr);
}
