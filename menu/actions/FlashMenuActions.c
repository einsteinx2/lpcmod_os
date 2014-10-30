/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "FlashMenuActions.h"

#include "include/boot.h"
#include "include/lpcmod_v1.h"
#include "BootIde.h"
#include "TextMenu.h"
#include "FlashMenuActions.h"

#include "boot.h"
#include "memory_layout.h"
#include "BootFATX.h"
#include "Gentoox.h"

extern void cromwellError(void);
extern void dots(void);

void FlashBiosFromHDD(void *fname) {
#ifdef FLASH
    int res;
    int offset;
    char * stringTemp;
    u8 * fileBuf;

    FATXPartition *partition;

    partition = OpenFATXPartition(0, SECTOR_SYSTEM, SYSTEM_SIZE);
    fileBuf = (u8 *)malloc(1024*1024);  //1MB buffer(max BIOS size)
    memset(fileBuf, 0x00, 1024*1024);   //Fill with 0.

    FATXFILEINFO fileinfo;
    //res = LoadFATXFilefixed(partition, fname, &fileinfo, (char*)0x100000);
    res = LoadFATXFilefixed(partition, fname, &fileinfo, fileBuf);
    if (!res) {
        printk("\n\n\n\n\n           Loading BIOS failed");
        dots();
        cromwellError();
        while(1);
    }

    offset = 0;
    if(fHasHardware == SYSCON_ID_V1){
        if(currentFlashBank == BNKOS){
            if(!ConfirmDialog("               Confirm update XBlast OS?", 1)){
                //res = BootReflashAndReset((char*)0x100000,offset,fileinfo.fileSize);
                res = BootReflashAndReset(fileBuf,offset,fileinfo.fileSize);
            }
            else
                res = -1;
        }
        else{
            if(currentFlashBank == BNK512)
                stringTemp = "             Confirm flash bank0(512KB)?";
            else
                stringTemp = "             Confirm flash bank1(256KB)?";
            if(!ConfirmDialog(stringTemp, 1)){
                //res = BootReflash((char*)0x100000,offset,fileinfo.fileSize);
                res = BootReflash(fileBuf,offset,fileinfo.fileSize);
            }
            else
                res = -1;
        }
    }
    else {
        if(!ConfirmDialog("               Confirm flash active bank?", 1)){
                //res = BootReflashAndReset((char*)0x100000,offset,fileinfo.fileSize);
            res = BootReflashAndReset(fileBuf,offset,fileinfo.fileSize);
        }
            else
                res = -1;
    }
    CloseFATXPartition(partition);
    free(fileBuf);
    if(res > 0) {
        cromwellError();
        printk("\n\n\n\n\n           Flash failed...");
        switch(res){
            case 1:
                printk("\n           ");
                cromwellError();
                printk("\n           Unknown flash device.\n           Write-Protect is enabled?");
                break;
            case 2:
                printk("\n           ");
                cromwellError();
                printk("\n           Cannot write to device.");
                break;
            case 3:
                printk("\n           ");
                cromwellError();
                printk("\n           File size error : %u", fileinfo.fileSize);
                break;
            case 4:
                printk("\n           ");
                cromwellError();
                printk("\n           Invalid XBlast OS update file.");
                break;
            case 5:
                printk("\n           ");
                cromwellError();
                printk("\n           CRC mismatch.");
                break;
            default:
                printk("\n           ");
                cromwellError();
                printk("\n           Unknown error! Congrats, you're not supposed to be here.");
                break;
        }
    }
    else if(res == -1){
        printk("\n\n\n\n\n\n\n\n\n\n\n           ");
        cromwellWarning();
        printk("\n           Flashing aborted.");
    }
    else if(res == -2){
        printk("\n\n\n\n\n\n\n\n\n\n\n           ");
        cromwellWarning();
        printk("\n           Erasing failed, please reflash.");
    }
    else if(res == -3){
        printk("\n\n\n\n\n\n\n\n\n\n\n           ");
        cromwellWarning();
        printk("\n           Programming failed, please reflash.");
    }
    else {
        printk("\n           ");
        cromwellSuccess();
        printk("\n           Flashing successful!!!");
    }
    FlashFooter();
    return;
#endif
}

void FlashBiosFromCD(void *cdromId) {
#ifdef FLASH
    extern unsigned char *videosavepage;
    memcpy((void*)FB_START,videosavepage,FB_SIZE);
    BootLoadFlashCD(*(int *)cdromId);
#endif
}

void enableNetflash(void *whatever) {
#ifdef FLASH
    extern unsigned char *videosavepage;
    memcpy((void*)FB_START,videosavepage,FB_SIZE);
    VIDEO_ATTR=0xffef37;
    printk("\n\n\n\n\n\n\n");
    VIDEO_ATTR=0xffc8c8c8;
    initialiseNetwork();
    netFlash();
#endif
}

void enableWebupdate(void *whatever) {
#ifdef FLASH
    extern unsigned char *videosavepage;
    memcpy((void*)FB_START,videosavepage,FB_SIZE);
    VIDEO_ATTR=0xffef37;
    printk("\n\n\n\n\n\n");
    VIDEO_ATTR=0xffc8c8c8;
    initialiseNetwork();
    webUpdate();
#endif
}

void switchBank(char bank)
{
    currentFlashBank = bank;
    xF70ELPCRegister  = bank;
    WriteToIO(XBLAST_CONTROL, bank);    // switch to proper bank
                                                                         //Send OSBNKCTRLBIT when toggling a bank other than BNKOS.
}    

void FlashFooter(void) {
    VIDEO_ATTR=0xffc8c8c8;
    printk("\n\n           Press Button 'A' to continue.");
    //if(fHasHardware == SYSCON_ID_V1)
    //    switchBank(BNKOS);
    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
    initialSetLED(LPCmodSettings.OSsettings.LEDColor);
}        
