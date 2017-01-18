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
#include "BootFATX.h"
#include "FlashDriver.h"
#include "Gentoox.h"
#include "string.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "lib/cromwell/cromString.h"
#include "LEDMenuActions.h"
#include "WebServerOps.h"

extern int etherboot(void);
extern int BootLoadFlashCD(int cdromId);

void FlashBiosFromHDD (void *fname) {
#ifdef FLASH
    int res;
    unsigned char * fileBuf;
    FATXFILEINFO fileinfo;
    FATXPartition *partition;

    partition = OpenFATXPartition (0, SECTOR_SYSTEM, SYSTEM_SIZE);
    fileBuf = (unsigned char *) malloc (1024 * 1024);  //1MB buffer(max BIOS size)
    memset (fileBuf, 0x00, 1024 * 1024);   //Fill with 0.
    
    //res = LoadFATXFilefixed(partition, fname, &fileinfo, (char*)0x100000);
    res = LoadFATXFile(partition, fname, &fileinfo);
    if (!res) {
        printk ("\n\n\n\n\n           Loading BIOS failed");
        dots ();
        cromwellError ();
        goto jumpToEnd;
    }
    memcpy(fileBuf, fileinfo.buffer, fileinfo.fileSize);
    free(fileinfo.buffer);
    fileinfo.buffer = fileBuf;
    FlashFileFromBuffer(fileinfo.buffer, fileinfo.fileSize, 1); //1 to display confirmDialog
    free(fileinfo.buffer);
jumpToEnd:
    CloseFATXPartition (partition);
    
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
    printk ("\n\n");
    VIDEO_ATTR = 0xffc8c8c8;

    if(nicInit == true || etherboot() == 0)
    {
        nicInit = true;
        extern int run_lwip(unsigned char flashType);
        debugSPIPrint("Starting network service\n");
        while(run_lwip(*(unsigned char *)flashType) == 0);
        debugSPIPrint("Killing network service\n");
        switch(currentWebServerOp)
        {
        case WebServerOps_BIOSFlash:
            FlashPrintResult();

            switchOSBank(FlashBank_OSBank);

            break;
        case WebServerOps_EEPROMFlash:
            UIFooter();
            break;
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
