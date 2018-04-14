/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "MenuInits.h"
#include "FlashMenuActions.h"
#include "boot.h"
#include "BootIde.h"
#include "memory_layout.h"
#include "FatFSAccessor.h"
#include "string.h"
#include "stdio.h"


void HDDFlashMenuDynamic(void* unused)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;

    DIREX dirHandle;
    FileInfo fileInfo;
    unsigned short n = 0;
    unsigned short bioses = 0;

    menuPtr = calloc(1, sizeof(TEXTMENU));

    strcpy(menuPtr->szCaption, getBIOSDirectoryLocation() + strlen("MASTER_"));

    if(isMounted(HDD_Master, Part_C))
    {
        dirHandle = fatxopendir(getBIOSDirectoryLocation());
        if(dirHandle)
        {
            do
            {
                fileInfo = fatxreaddir(dirHandle);
                if(0 == fileInfo.nameLength)
                {
                   break;
                }
                n++;
                // Check the file.
                if((fileInfo.size % (256 * 1024) == 0))
                {
                    // If it's a (readable) file - i.e. not a directory.
                    // AND it's filesize is divisible by 256k.
                    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                    strcpy(itemPtr->szCaption, fileInfo.name);
                    itemPtr->functionPtr = FlashBiosFromHDD;
                    itemPtr->functionDataPtr = itemPtr->szCaption;
                    TextMenuAddItem(menuPtr, itemPtr);
                    bioses++;
                }
            } while(1);

            if(0 == n)
            {
                // If there were no directories and no files.
                itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                sprintf(itemPtr->szCaption, "No files in %s.", getBIOSDirectoryLocation() + strlen("MASTER_"));
                itemPtr->functionPtr = NULL;
                TextMenuAddItem(menuPtr, itemPtr);
            }
            else if(0 == bioses)
            {
                // If there were directories, but no files.
                itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                sprintf(itemPtr->szCaption, "No BIOS files in %s.", getBIOSDirectoryLocation() + strlen("MASTER_"));
                itemPtr->functionPtr = NULL;
                TextMenuAddItem(menuPtr, itemPtr);
            }

            fatxclosedir(dirHandle);
        }
        else
        {
            // If C:\BIOS doesnt exist.
            itemPtr = calloc(1, sizeof(TEXTMENUITEM));
            sprintf(itemPtr->szCaption, "%s does not exist.", getBIOSDirectoryLocation() + strlen("MASTER_"));
            itemPtr->functionPtr = NULL;
            TextMenuAddItem(menuPtr, itemPtr);
        }
    }
    else
    {
        // If the partition couldn't be opened at all.
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption, "Error reading C:\\ partition.");
        itemPtr->functionPtr = NULL;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    ResetDrawChildTextMenu(menuPtr);
}
