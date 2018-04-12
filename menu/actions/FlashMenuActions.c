/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "FlashMenuActions.h"
#include "MenuActions.h"
#include "lpcmod_v1.h"
#include "FlashUi.h"
#include "BootIde.h"
#include "TextMenu.h"
#include "boot.h"
#include "video.h"
#include "memory_layout.h"
#include "FatFSAccessor.h"
#include "FlashDriver.h"
#include "Gentoox.h"
#include "string.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "lib/cromwell/cromString.h"
#include "lib/cromwell/cromSystem.h"
#include "LEDMenuActions.h"
#include "WebServerOps.h"

extern int etherboot(void);
extern int BootLoadFlashCD(int cdromId);

static const char* const biosDirectoryLocation = "MASTER_C:\\BIOS\\";

const char* const getBIOSDirectoryLocation(void)
{
    return biosDirectoryLocation;
}

void FlashBiosFromHDD (void *fname) {
#ifdef FLASH
    int res = 0;
    unsigned char fileBuf[1024 * 1024];
    unsigned int size;
    const char* filename = (const char *)fname;
    char fullPathName[255 + sizeof('\0')];
    FILEX fileHandle;

    if(255 < (strlen(getBIOSDirectoryLocation()) + strlen(filename)))
    {
        return;
    }

    sprintf(fullPathName, "%s%s", getBIOSDirectoryLocation(), filename);

    memset (fileBuf, 0x00, 1024 * 1024);   //Fill with 0.
    fileHandle = fatxopen(fullPathName, FileOpenMode_OpenExistingOnly | FileOpenMode_Read);
    if(fileHandle)
    {
        size = fatxsize(fileHandle);
        if((1024 * 1024) >= size)
        {
            if(size == fatxread(fileHandle, fileBuf, size))
            {
                res = 1;
            }
        }
        fatxclose(fileHandle);
    }
    
    if (0 == res)
    {
        printk ("\n\n\n\n\n           Loading BIOS failed");
        dots ();
        cromwellError ();
    }
    else
    {
        FlashFileFromBuffer(fileBuf, size, 1); //1 to display confirmDialog
    }
    
    return;
#endif
}

void FlashBiosFromCD (void *cdromId) {
#ifdef FLASH
    BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
    BootLoadFlashCD (*(int *) cdromId);
#endif
}

void enableNetflash (void *flashType) {
#ifdef FLASH
    static bool nicInit = false;
    BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
    printk ("\n\n            Starting network interface. ");
    VIDEO_ATTR = 0xffc8c8c8;

    if(nicInit == true || etherboot() == 0)
    {
        nicInit = true;
        cromwellSuccess();
        debugSPIPrint(DEBUG_GENERAL_UI, "Starting network service\n");
        startNetFlash(*(WebServerOps *)flashType);
        while(cromwellLoop())
        {
            if(netflashPostProcess())
            {
                debugSPIPrint(DEBUG_GENERAL_UI, "Killing network service\n");
                break;
            }
        }

    }
#endif
}

void enableWebupdate (void *whatever) {
#ifdef FLASH
    BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
    printk ("\n\n");
    VIDEO_ATTR = 0xffc8c8c8;

    //initialiseNetwork ();
    //webUpdate ();
#endif
}

void FlashFooter(void)
{
    UIFooter();
    initialSetLED (LPCmodSettings.OSsettings.LEDColor);
}
