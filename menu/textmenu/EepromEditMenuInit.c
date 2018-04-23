/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "MenuInits.h"
#include "EepromEditMenuActions.h"
#include "lpcmod_v1.h"
#include "FlashMenuActions.h"
#include "string.h"
#include "stdio.h"
#include "FatFSAccessor.h"
#include "WebServerOps.h"

TEXTMENU* EEPROMFileRestoreMenuInit(void);

TEXTMENU* eepromEditMenuInit(void)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "Edit EEPROM content");

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Show edited EEPROM image content");
    itemPtr->functionPtr = displayEditEEPROMBuffer;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);
/*
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Edit HDD password");
    itemPtr->functionPtr=NULL;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Edit Serial number");
    itemPtr->functionPtr=NULL;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);
*/
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Edit MAC address");
    itemPtr->functionPtr = editMACAddress;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);
/*
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Edit Online key");
    itemPtr->functionPtr=NULL;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);
*/
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Restore from file");
    itemPtr->functionPtr = dynamicDrawChildTextMenu;
    itemPtr->functionDataPtr = EEPROMFileRestoreMenuInit;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Restore from network");
    itemPtr->functionPtr = enableNetflash;
    itemPtr->functionDataPtr = malloc(sizeof(WebServerOps));
    *(WebServerOps *)itemPtr->functionDataPtr= WebServerOps_EEPROMFlash;
    itemPtr->dataPtrAlloc = true;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Brute Force Fix EEPROM");
    itemPtr->functionPtr = bruteForceFixDisplayresult;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Last resort recovery");
    itemPtr->functionPtr = LastResortRecovery;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Save to EEPROM and reboot");
    itemPtr->functionPtr = confirmSaveToEEPROMChip;
    itemPtr->functionDataPtr = NULL;
    TextMenuAddItem(menuPtr, itemPtr);

    return menuPtr;
}

TEXTMENU* EEPROMFileRestoreMenuInit(void)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;

    DIREX dirHandle;
    FileInfo fileInfo;
    unsigned short n = 0;
    unsigned short eeproms = 0;

    menuPtr = calloc(1, sizeof(TEXTMENU));

    strcpy(menuPtr->szCaption, getEEPROMDirectoryLocation() + strlen("MASTER_"));

    debugSPIPrint(DEBUG_EEPROM_DRIVER, "Listing eeproms in %s\n", getEEPROMDirectoryLocation());

    if(isMounted(HDD_Master, Part_C))
    {
        dirHandle = fatxopendir(getEEPROMDirectoryLocation());
        if(dirHandle)
        {
            do
            {
                fileInfo = fatxreaddir(dirHandle);
                if(0 == fileInfo.nameLength || '\0' == fileInfo.name[0])
                {
                   break;
                }
                n++;
                // Check the file.
                if(fileInfo.size  == 256)
                {
                    // If it's a (readable) file - i.e. not a directory.
                    // AND it's filesize is divisible by 256k.
                    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                    strcpy(itemPtr->szCaption, fileInfo.name);
                    itemPtr->functionPtr = restoreEEPROMFromFile;
                    itemPtr->functionDataPtr = itemPtr->szCaption;
                    TextMenuAddItemInOrder(menuPtr, itemPtr);
                    eeproms++;
                }
            } while(1);

            if(0 == n)
            {
                // If there were no directories and no files.
                itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                sprintf(itemPtr->szCaption, "No files in %s.", getEEPROMDirectoryLocation() + strlen("MASTER_"));
                itemPtr->functionPtr = NULL;
                TextMenuAddItem(menuPtr, itemPtr);
            }
            else if(0 == eeproms)
            {
                // If there were directories, but no files.
                itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                sprintf(itemPtr->szCaption, "No EEPROM files in %s.", getEEPROMDirectoryLocation() + strlen("MASTER_"));
                itemPtr->functionPtr = NULL;
                TextMenuAddItem(menuPtr, itemPtr);
            }

            fatxclosedir(dirHandle);
        }
        else
        {
            // If C:\XBlast\eeproms doesnt exist.
            itemPtr = calloc(1, sizeof(TEXTMENUITEM));
            sprintf(itemPtr->szCaption, "%s does not exist.", getEEPROMDirectoryLocation() +  + strlen("MASTER_"));
            itemPtr->functionPtr = NULL;
            TextMenuAddItem(menuPtr, itemPtr);
        }

    }
    else
    {
        // If the partition couldn't be opened at all.
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption,"Error reading C:\\ partition.");
        itemPtr->functionPtr = NULL;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    return menuPtr;
}
