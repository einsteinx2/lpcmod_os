/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "boot.h"
#include "video.h"
#include "memory_layout.h"
#include <shared.h>
#include <filesys.h>
#include "rc4.h"
#include "sha1.h"
#include "BootFATX.h"
#include "xbox.h"
#include "BootFlash.h"
#include "cpu.h"
#include "VideoInitialization.h"
#include "TextMenu.h"
#include "lpcmod_v1.h"
CONFIGENTRY *LoadConfigCD(int);
//TEXTMENU *TextMenuInit(void);

void AdvancedMenu(void *textmenu) {
    TextMenu((TEXTMENU*)textmenu, NULL);
}

// Booting Original Bios
void BootOriginalBios(void *data) {
    BootFlashSaveOSSettings();
    assertWriteEEPROM();
    
    BootStopUSB();
    
    if(fHasHardware == SYSCON_ID_V1 && cromwell_config==CROMWELL){
        WriteToIO(DISABLE_MOD, *(u8*)data);    // switch to original bios
        I2CTransmitWord(0x10, 0x1b00 + ( I2CTransmitByteGetReturn(0x10, 0x1b) & 0xfb )); // clear noani-bit
    }
    else {
        I2CTransmitWord(0x10, 0x1b00 + ( I2CTransmitByteGetReturn(0x10, 0x1b) | 0x04 )); // set noani-bit
    }
    I2CRebootQuick();
    while(1);
}    

// Booting bank Modbios
void BootModBios(void *data) {
    BootFlashSaveOSSettings();
    assertWriteEEPROM();
    switchBank(*(u8*)data);
    
    BootStopUSB();
    
    if(cromwell_config==CROMWELL)
      I2CTransmitWord(0x10, 0x1b00 + ( I2CTransmitByteGetReturn(0x10, 0x1b) & 0xfb )); // clear noani-bit
    else
      I2CTransmitWord(0x10, 0x1b00 + ( I2CTransmitByteGetReturn(0x10, 0x1b) | 0x04 )); // set noani-bit
    I2CRebootQuick();
    while(1);
}

void BootFromCD(void *data) {
    BootFlashSaveOSSettings();
    assertWriteEEPROM();
    //We have to go an extra step when the CD icon is selected, as unlike
    //the other boot modes, we have not parsed the linuxboot.cfg file yet.
    int nTempCursorY = VIDEO_CURSOR_POSY; 
    CONFIGENTRY *config = LoadConfigCD(*(int*)data);
    if (config==NULL) {
        errorLED();
        printk("\n\n           Could not boot from disc!\n           Try different media and a lower burning speed.\n");
        wait_ms(5000);
        inputLED();
        //Clear the screen and return to the menu
        BootVideoClearScreen(&jpegBackdrop, nTempCursorY, VIDEO_CURSOR_POSY+1);    
        return;
    }
    DrawBootMenu(config);
}
/*
void BootFromNet(void *whatever) {
   extern unsigned char *videosavepage;
   memcpy((void*)FB_START,videosavepage,FB_SIZE);
   VIDEO_ATTR=0xffef37;
   VIDEO_ATTR=0xffc8c8c8;
   initialiseNetwork();
   netBoot();
}
*/

void DrawBootMenu(void *rootEntry) {
    //entry is the pointer to the root config entry
    TEXTMENU *menu;
    TEXTMENUITEM *menuPtr, *defaultMenuItem;
    CONFIGENTRY *configEntry, *currentConfigEntry;
    extern int timedOut;

    defaultMenuItem=NULL;
    configEntry = rootEntry;

    if (configEntry->nextConfigEntry==NULL) {
        //If there is only one option, just boot it.
        BootMenuEntry(configEntry);
        return;
    }

    if (timedOut) {
        //We should be non-interactive, then.
        //If there is a default entry, boot that.
        for (currentConfigEntry = configEntry; currentConfigEntry != NULL; 
            currentConfigEntry = currentConfigEntry->nextConfigEntry) {
            if (currentConfigEntry->isDefault) {
                BootMenuEntry(currentConfigEntry);
                return;
            }
        }
        //There wasn't a default entry, so just boot the first in the list
        BootMenuEntry(configEntry);
        return;
    }
    
    menu = malloc(sizeof(TEXTMENU));
    memset(menu,0x00,sizeof(TEXTMENU));
    strcpy(menu->szCaption, "Boot menu");
  
    for (currentConfigEntry = configEntry; currentConfigEntry != NULL; 
        currentConfigEntry = currentConfigEntry->nextConfigEntry) {
    
        menuPtr = (TEXTMENUITEM *)malloc(sizeof(TEXTMENUITEM*));
        memset(menuPtr, 0x00, sizeof(menuPtr));
        if (currentConfigEntry->title == NULL) {
            strcpy(menuPtr->szCaption,"Untitled");
        } else { 
            strncpy(menuPtr->szCaption,currentConfigEntry->title,50);
        }
        menuPtr->functionPtr = BootMenuEntry;
        menuPtr->functionDataPtr = (void *)currentConfigEntry;
        //If this config entry is default, mark the menu item as default.
        if (currentConfigEntry->isDefault) defaultMenuItem = menuPtr;
        TextMenuAddItem(menu,menuPtr);
    }
    TextMenu(menu, defaultMenuItem);
}

void BootMenuEntry(void *entry) {
    CONFIGENTRY *config = (CONFIGENTRY*)entry;
    if (!(config->nextConfigEntry==NULL) || !(config->previousConfigEntry==NULL)) {
        extern unsigned char *videosavepage;
        memcpy((void*)FB_START,videosavepage,FB_SIZE);
    }

    switch (config->bootType) {
        case BOOT_CDROM:
            LoadKernelCdrom(config);
            break;
        case BOOT_FATX:
            LoadKernelFatX(config);
            break;
        case BOOT_NATIVE:
            LoadKernelNative(config);
            break;
    }
    ExittoLinux(config);
}

void DrawChildTextMenu(void *menu) {
    TextMenu((TEXTMENU*)menu);
}

void ResetDrawChildTextMenu(void *menu) {
    TEXTMENU * resetSelection = (TEXTMENU*)menu;
    TextMenu((TEXTMENU*)menu, resetSelection->firstMenuItem);
}

void DrawLargeHDDTextMenu(u8 drive){
    TEXTMENU *menuPtr;
    breakOutOfMenu = 1;
    menuPtr = (TEXTMENU *)LargeHDDMenuInit((void *)&drive);
    
    //Free memory
    if(menuPtr->firstMenuItem != NULL){
        if(menuPtr->firstMenuItem->nextMenuItem != NULL){
            if(menuPtr->firstMenuItem->nextMenuItem->nextMenuItem != NULL){
                if(menuPtr->firstMenuItem->nextMenuItem->nextMenuItem->nextMenuItem != NULL){
                    free(menuPtr->firstMenuItem->nextMenuItem->nextMenuItem->nextMenuItem->functionDataPtr);
                    free(menuPtr->firstMenuItem->nextMenuItem->nextMenuItem->nextMenuItem);
                }
                free(menuPtr->firstMenuItem->nextMenuItem->nextMenuItem->functionDataPtr);
                free(menuPtr->firstMenuItem->nextMenuItem->nextMenuItem);
            }
            free(menuPtr->firstMenuItem->nextMenuItem->functionDataPtr);
            free(menuPtr->firstMenuItem->nextMenuItem);
        }
        free(menuPtr->firstMenuItem->functionDataPtr);
        free(menuPtr->firstMenuItem);
    }
    free(menuPtr);
}

#ifdef ETHERBOOT 
extern int etherboot(void);
void BootFromEtherboot(void *data) {
    busyLED();
    initialiseNetwork();
    etherboot();
}
#endif

#ifdef FLASH
/*
void FlashBios(void *data) {
    BootFlashSaveOSSettings();
    assertWriteEEPROM();
    BootLoadFlashCD();
}
*/
#endif

