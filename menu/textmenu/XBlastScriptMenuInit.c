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
#include "boot.h"
#include "BootIde.h"
#include "memory_layout.h"
#include "FatFSAccessor.h"
#include "lpcmod_v1.h"
#include "string.h"
#include "stdio.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "xblast/HardwareIdentifier.h"
#include "FatFSAccessor.h"

TEXTMENU* RunScriptMenuInit(void);
TEXTMENU* SaveScriptMenuInit(void);


TEXTMENU* XBlastScriptMenuInit(void)
{
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "XBlast scripts");

    //Run Script.
    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Run script from HDD");
    itemPtr->functionPtr = dynamicDrawChildTextMenu;
    itemPtr->functionDataPtr = RunScriptMenuInit;
    TextMenuAddItem(menuPtr, itemPtr);

    //Save script to flash.
    if(isXBE() == false || isXBlastOnLPC())
    {
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Save script to flash");
        itemPtr->functionPtr = dynamicDrawChildTextMenu;
        itemPtr->functionDataPtr = SaveScriptMenuInit;
        TextMenuAddItem(menuPtr, itemPtr);

        if(LPCmodSettings.flashScript.scriptSize > 0)
        {
            itemPtr = calloc(1, sizeof(TEXTMENUITEM));
            strcpy(itemPtr->szCaption, "Erase script from flash");
            itemPtr->functionPtr = deleteFlashScriptFromFlash;
            itemPtr->functionDataPtr = NULL;
            TextMenuAddItem(menuPtr, itemPtr);
        }
    }

    //Load script from flash.
    if(LPCmodSettings.flashScript.scriptSize > 0)
    {
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Run script from flash");
        itemPtr->functionPtr = loadScriptFromFlash;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    //No need to show this item if there's no way to persist setting.
    if(isXBE() == false || isXBlastOnLPC())
    {
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Enable Boot script : ");
        strcpy(itemPtr->szParameter, LPCmodSettings.OSsettings.runBootScript? "Yes" : "No");
        itemPtr->functionPtr = toggleRunBootScript;
        itemPtr->functionDataPtr= itemPtr->szParameter;
        itemPtr->functionLeftPtr=toggleRunBootScript;
        itemPtr->functionLeftDataPtr = itemPtr->szParameter;
        itemPtr->functionRightPtr=toggleRunBootScript;
        itemPtr->functionRightDataPtr = itemPtr->szParameter;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Enable Bank script : ");
    strcpy(itemPtr->szParameter, LPCmodSettings.OSsettings.runBankScript? "Yes" : "No");
    itemPtr->functionPtr = toggleRunBankScript;
    itemPtr->functionDataPtr= itemPtr->szParameter;
    itemPtr->functionLeftPtr=toggleRunBankScript;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=toggleRunBankScript;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    return menuPtr;

}

TEXTMENU* RunScriptMenuInit(void)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;

    DIREX dirHandle;
    FileInfo fileInfo;
    unsigned short n = 0;

    XBlastLogger(DBG_LVL_DEBUG, DEBUG_SCRIPT, "Listing scripts in %s", getScriptDirectoryLocation());

    menuPtr = calloc(1, sizeof(TEXTMENU));

    strcpy(menuPtr->szCaption, getScriptDirectoryLocation() + strlen("MASTER_"));

    if(isMounted(HDD_Master, Part_C))
    {
        dirHandle = fatxopendir(getScriptDirectoryLocation());
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
                if(0 < fileInfo.size)
                {
                    // If it's a (readable) file - i.e. not a directory.
                    // AND it's filesize is divisible by 256k.
                    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                    strcpy(itemPtr->szCaption, fileInfo.name);
                    itemPtr->functionPtr = loadRunScriptNoParams;
                    itemPtr->functionDataPtr = itemPtr->szCaption;
                    TextMenuAddItemInOrder(menuPtr, itemPtr);
                }
            } while(1);

            if(0 == n)
            {
                // If there were no directories and no files.
                itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                sprintf(itemPtr->szCaption, "No files in %s.", getScriptDirectoryLocation() + strlen("MASTER_"));
                itemPtr->functionPtr = NULL;
                TextMenuAddItem(menuPtr, itemPtr);
            }

            fatxclosedir(dirHandle);
        }
        else
        {
            // If C:\XBlast\Scripts doesnt exist.
            itemPtr = calloc(1, sizeof(TEXTMENUITEM));
            sprintf(itemPtr->szCaption, "%s does not exist.", getScriptDirectoryLocation() + strlen("MASTER_"));
            itemPtr->functionPtr = NULL;
            TextMenuAddItem(menuPtr, itemPtr);
        }
    }
    else
    {
        // If the partition couldn't be opened at all.
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Error reading C:\\ partition.");
        itemPtr->functionPtr = NULL;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    return menuPtr;
}

TEXTMENU* SaveScriptMenuInit(void)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;

    DIREX dirHandle;
    FileInfo fileInfo;
    unsigned short n = 0;
    
    XBlastLogger(DBG_LVL_DEBUG, DEBUG_SCRIPT, "Listing scripts in %s", getScriptDirectoryLocation());

    menuPtr = calloc(1, sizeof(TEXTMENU));

    strcpy(menuPtr->szCaption, getScriptDirectoryLocation() + strlen("MASTER_"));

    if(isMounted(HDD_Master, Part_C))
    {
        dirHandle = fatxopendir(getScriptDirectoryLocation());
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
                if(0 < fileInfo.size)
                {
                    // If it's a (readable) file - i.e. not a directory.
                    // AND it's filesize is divisible by 256k.
                    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                    strcpy(itemPtr->szCaption, fileInfo.name);
                    itemPtr->functionPtr = saveScriptToFlash;
                    itemPtr->functionDataPtr = itemPtr->szCaption;
                    TextMenuAddItemInOrder(menuPtr, itemPtr);
                }
            } while(1);

            if(0 == n)
            {
                // If there were no directories and no files.
                itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                sprintf(itemPtr->szCaption, "No files in %s.", getScriptDirectoryLocation() + strlen("MASTER_"));
                itemPtr->functionPtr = NULL;
                TextMenuAddItem(menuPtr, itemPtr);
            }

            fatxclosedir(dirHandle);
        }
        else
        {
            // If C:\XBlast\Scripts doesnt exist.
            itemPtr = calloc(1, sizeof(TEXTMENUITEM));
            sprintf(itemPtr->szCaption, "%s does not exist.", getScriptDirectoryLocation() + strlen("MASTER_"));
            itemPtr->functionPtr = NULL;
            TextMenuAddItem(menuPtr, itemPtr);
        }
    }
    else
    {
        // If the partition couldn't be opened at all.
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Error reading C:\\ partition.");
        itemPtr->functionPtr = NULL;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    return menuPtr;
}
