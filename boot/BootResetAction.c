/*
 * Sequences the necessary post-reset actions from as soon as we are able to run C
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
 */

#include "boot.h"
#include "BootEEPROM.h"
#include "BootFlash.h"
#include "BootFATX.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "xbox.h"
#include "cpu.h"
#include "config.h"
#include "video.h"
#include "memory_layout.h"
#include "lpcmod_v1.h"
//#include "lib/LPCMod/BootLCD.h"

JPEG jpegBackdrop;

int nTempCursorMbrX, nTempCursorMbrY;

extern volatile int nInteruptable;

volatile CURRENT_VIDEO_MODE_DETAILS vmode;
//extern KNOWN_FLASH_TYPE aknownflashtypesDefault[];

void ClearScreen (void) {
    BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
}

//////////////////////////////////////////////////////////////////////
//
//  BootResetAction()

extern void BootResetAction ( void ) {
    bool fMbrPresent=false;
    bool fSeenActive=false;
    int nFATXPresent=false;
    bool fFirstBoot=false;                    //Flag to indicate first boot since flash update
    int nTempCursorX, nTempCursorY;
    int n, nx;
    char *modName = "Unsupported modchip!";
    OBJECT_FLASH of;


    //Length of array is set depending on how many revision can be uniquely identified.
    //Modify this enum if you modify the "XBOX_REVISION" enum in boot.h
    char *xbox_mb_rev[8] = {
        "DevKit",
        "DebugKit",
        "1.0",
        "1.1",
        "1.2/1.3",
        "1.4/1.5",
        "1.6/1.6b",
        "Unknown"
    };

    fHasHardware = 0;

    memcpy(&cromwell_config,(void*)(0x03A00000+0x20),4);
    memcpy(&cromwell_retryload,(void*)(0x03A00000+0x24),4);
    memcpy(&cromwell_loadbank,(void*)(0x03A00000+0x28),4);
    memcpy(&cromwell_Biostype,(void*)(0x03A00000+0x2C),4);

    VIDEO_CURSOR_POSX=40;
    VIDEO_CURSOR_POSY=140;
        
    VIDEO_AV_MODE = 0xff;
    nInteruptable = 0;

    // prep our BIOS console print state
    VIDEO_ATTR=0xffffffff;

    // init malloc() and free() structures
    MemoryManagementInitialization((void *)MEMORYMANAGERSTART, MEMORYMANAGERSIZE);

    BootInterruptsWriteIdt();

    // initialize the PCI devices
    //bprintf("BOOT: starting PCI init\n\r");
    BootPciPeripheralInitialization();
    // Reset the AGP bus and start with good condition
    BootAGPBUSInitialization();

    LEDRed();        //Signal the user to press Eject button to avoid Quickboot.
    if(cromwell_config==CROMWELL){              //Only check if booted from ROM.
        fHasHardware = LPCMod_HW_rev();         //Will output 0 if no supported modchip detected.
    }
    if(fHasHardware){        //Don't try to read from flash if none is detected
        if(fHasHardware == SYSCON_ID_V1){
            sprintf(modName,"%s", "XBlast Lite V1");
            //Make sure we'll be reading from OS Bank
            switchBank(BNKOS);
        }
        else {
            if(fHasHardware == SYSCON_ID_XX1 || fHasHardware == SYSCON_ID_XX2)
                sprintf(modName,"%s", "SmartXX V1/V2");
            else if(fHasHardware == SYSCON_ID_XXOPX)
                sprintf(modName,"%s", "SmartXX LT OPX");
            else if(fHasHardware == SYSCON_ID_XX3)
                sprintf(modName,"%s", "SmartXX V3");

            currentFlashBank = BNKOS;           //Make sure the system knows we're on the right bank.
        }
        //Retrieve XBlast OS settings from flash
        BootFlashGetOSSettings(&LPCmodSettings);
    }

    if(LPCmodSettings.OSsettings.migrateSetttings == 0xFF ||
       LPCmodSettings.OSsettings.activeBank == 0xFF ||
       LPCmodSettings.OSsettings.altBank == 0xFF ||
       LPCmodSettings.OSsettings.Quickboot == 0xFF ||
       LPCmodSettings.OSsettings.selectedMenuItem == 0xFF ||
       LPCmodSettings.OSsettings.fanSpeed == 0xFF ||
       LPCmodSettings.OSsettings.bootTimeout == 0xFF ||
       LPCmodSettings.OSsettings.LEDColor == 0xFF ||
       LPCmodSettings.OSsettings.TSOPcontrol == 0xFF ||
       LPCmodSettings.OSsettings.enableNetwork == 0xFF ||
       LPCmodSettings.OSsettings.useDHCP == 0xFF ||
       LPCmodSettings.LCDsettings.migrateLCD == 0xFF ||
       LPCmodSettings.LCDsettings.enable5V == 0xFF ||
       LPCmodSettings.LCDsettings.lcdType == 0xFF ||
       LPCmodSettings.LCDsettings.nbLines == 0xFF ||
       LPCmodSettings.LCDsettings.lineLength == 0xFF ||
       LPCmodSettings.LCDsettings.backlight == 0xFF ||
       LPCmodSettings.LCDsettings.contrast == 0xFF ||
       LPCmodSettings.LCDsettings.displayMsgBoot == 0xFF ||
       LPCmodSettings.LCDsettings.customTextBoot == 0xFF ||
       LPCmodSettings.LCDsettings.displayBIOSNameBoot == 0xFF){
            fFirstBoot = true;
            initialLPCModOSBoot(&LPCmodSettings);                //No settings for LPCMod were present in flash.
            //OS sometimes lock on after a fresh flash. Disabling to see if that's causing it.(probably)
            //BootFlashSaveOSSettings();        //Put some initial values in there.
            LEDFirstBoot(NULL);
            LPCmodSettings.OSsettings.bootTimeout = 0;        //No countdown since it's the first boot since a flash update.
                                                            //Configure your device first.
    }
    if(fHasHardware){
    	//I2CSetFanSpeed(LPCmodSettings.OSsettings.fanSpeed);
    	I2CSetFanSpeed(LPCmodSettings.OSsettings.fanSpeed);
    }
    else {
    	LPCmodSettings.OSsettings.fanSpeed = I2CGetFanSpeed();
    }

    BootLCDInit();                              //Basic init. Do it even if no LCD is connected on the system.
    
    //Stuff to do right after loading persistent settings from flash.
    if(!fFirstBoot){                                        //No need to change fan speed on first boot.
        if(fHasHardware == SYSCON_ID_V1 ||
           fHasHardware == SYSCON_ID_XX1 ||
           fHasHardware == SYSCON_ID_XX2 ||
           fHasHardware == SYSCON_ID_XXOPX ||
           fHasHardware == SYSCON_ID_XX3){
            assertInitLCD();                            //Function in charge of checking if a init of LCD is needed.
        }
        //further init here.
    }



    // We disable The CPU Cache
    cache_disable();
    // We Update the Microcode of the CPU
    display_cpuid_update_microcode();
    // We Enable The CPU Cache
    cache_enable();
    //setup_ioapic();
    // We look how much memory we have ..
    BootDetectMemorySize();

    BootEepromReadEntireEEPROM();
        
    I2CTransmitWord(0x10, 0x1a01); // unknown, done immediately after reading out eeprom data
    I2CTransmitWord(0x10, 0x1b04); // unknown
        
    /* Here, the interrupts are Switched on now */
    BootPciInterruptEnable();
    /* We allow interrupts */
    nInteruptable = 1;
    

#ifndef SILENT_MODE
    printk("           BOOT: start USB init\n");
#endif
    BootStartUSB();

    //Load up some more custom settings right before booting to OS.
    if(!fFirstBoot){
        if(fHasHardware == SYSCON_ID_V1){                       //Quickboot only if on the right hardware.
            wait_ms(550);
            if(XPAD_current[0].keys[4] == 1){                   //Black button pressed.
                BootModBios(&(LPCmodSettings.OSsettings.altBank));
            }
            if(XPAD_current[0].keys[5] == 0 && LPCmodSettings.OSsettings.Quickboot == 1){       //White button NOT pressed and Quickboot ON.
                BootModBios(&(LPCmodSettings.OSsettings.activeBank));
            }
        }
        initialSetLED(LPCmodSettings.OSsettings.LEDColor);
    }
    // Load and Init the Background image
    // clear the Video Ram
    memset((void *)FB_START,0x00,0x400000);

    BootVgaInitializationKernelNG((CURRENT_VIDEO_MODE_DETAILS *)&vmode);

    { // decode and malloc backdrop bitmap
        extern int _start_backdrop;
        BootVideoJpegUnpackAsRgb(
            (u8 *)&_start_backdrop,
            &jpegBackdrop
        );
    }
    // paint the backdrop
#ifndef DEBUG_MODE
    BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
#endif

    /* Here, the interrupts are Switched on now */
//    BootPciInterruptEnable();
        /* We allow interrupts */
//    nInteruptable = 1;

    I2CTransmitWord(0x10, 0x1901); // no reset on eject
         
    VIDEO_CURSOR_POSX=(vmode.xmargin/*+64*/)*4;
    VIDEO_CURSOR_POSY=vmode.ymargin;

    printk("\n\n");
    if (cromwell_config==XROMWELL) {
        printk("           \2XBlast OS (XBE) v" VERSION "\n\n\2");
    } else if (cromwell_config==CROMWELL) {
        printk("           \2XBlast OS (ROM) v" VERSION "\n\n\2");
    }

    VIDEO_ATTR=0xff00ff00;

    VIDEO_CURSOR_POSX=(vmode.xmargin/*+64*/)*4;
    VIDEO_CURSOR_POSY=vmode.ymargin+64;


    VIDEO_ATTR=0xff00ff00;
    printk("           Modchip: %s    DEBUG_fHasHardware: 0x%02x\n",modName, fHasHardware);
    VIDEO_ATTR=0xffc8c8c8;
    printk("           THIS IS A WIP BUILD\n ");

    VIDEO_ATTR=0xff00ff00;
    

   mbVersion = I2CGetXboxMBRev();
   printk("           Xbox revision: %s ", xbox_mb_rev[mbVersion]);
   if (xbox_ram > 64) {
        VIDEO_ATTR=0xff00ff00;
   } else {
        VIDEO_ATTR=0xffffa20f;
   }
   printk("  RAM: %dMiB\n", xbox_ram);
   
    VIDEO_CURSOR_POSX=(vmode.xmargin/*+64*/)*4;
#ifndef SILENT_MODE
    // capture title area
    VIDEO_ATTR=0xffc8c8c8;
    printk("           Encoder: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%s  ", VideoEncoderName());
    VIDEO_ATTR=0xffc8c8c8;
    printk("Cable: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%s  ", AvCableName());
        
    if (I2CGetTemperature(&n, &nx)) {
        VIDEO_ATTR=0xffc8c8c8;
        printk("CPU Temp: ");
        VIDEO_ATTR=0xffc8c800;
        printk("%doC  ", n);
        VIDEO_ATTR=0xffc8c8c8;
        printk("M/b Temp: ");
        VIDEO_ATTR=0xffc8c800;
        printk("%doC  ", nx);
    }

    printk("\n");
    nTempCursorX=VIDEO_CURSOR_POSX;
    nTempCursorY=VIDEO_CURSOR_POSY;
#endif

    VIDEO_ATTR=0xffffffff;

    //busyLED();

    // set Ethernet MAC address from EEPROM
    {
        volatile u8 * pb=(u8 *)0xfef000a8;  // Ethernet MMIO base + MAC register offset (<--thanks to Anders Gustafsson)
        int n;
        for(n=5;n>=0;n--) { *pb++=    eeprom.MACAddress[n]; } // send it in backwards, its reversed by the driver
    }
#ifndef SILENT_MODE
    BootEepromPrintInfo();
#endif

    // init the IDE devices
#ifndef SILENT_MODE
    VIDEO_ATTR=0xffc8c8c8;
    printk("           Initializing IDE Controller\n");
#endif
    BootIdeWaitNotBusy(0x1f0);
    wait_ms(100);
#ifndef SILENT_MODE
    printk("           Ready\n");
#endif
    // reuse BIOS status area

#ifndef DEBUG_MODE
//    BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
#endif
    VIDEO_CURSOR_POSX=nTempCursorX;
    VIDEO_CURSOR_POSY=nTempCursorY;
    VIDEO_CURSOR_POSX=0;
    VIDEO_CURSOR_POSY=0;

    BootIdeInit();

    printk("\n\n\n\n");

    nTempCursorMbrX=VIDEO_CURSOR_POSX;
    nTempCursorMbrY=VIDEO_CURSOR_POSY;
    // We save the complete framebuffer to memory (we restore at exit)
    u8 *videosavepage = malloc(FB_SIZE);
    memcpy(videosavepage,(void*)FB_START,FB_SIZE);
    
    //Check for unformatted drives.
    int i;
    for (i=0; i<2; ++i) {
        if (tsaHarddiskInfo[i].m_fDriveExists && !tsaHarddiskInfo[i].m_fAtapi) {
            if(!FATXCheckBRFR(i)){
                char * ConfirmDialogString;
                sprintf(ConfirmDialogString, "               Format new drive (%s)?", i ? "slave":"master");
                if(!ConfirmDialog(ConfirmDialogString, 1)){
                    FATXFormatDriveC(i);
                    FATXFormatDriveE(i);
                    FATXFormatCacheDrives(i);
                    FATXSetBRFR(i);
                }
            }
        }
    }
    memcpy((void*)FB_START,videosavepage,FB_SIZE);
    free(videosavepage);

//    printk("i2C=%d SMC=%d, IDE=%d, tick=%d una=%d unb=%d\n", nCountI2cinterrupts, nCountInterruptsSmc, nCountInterruptsIde, BIOS_TICK_COUNT, nCountUnusedInterrupts, nCountUnusedInterruptsPic2);
    IconMenuInit();
    //inputLED();
    IconMenu();

    //Should never come back here.
    while(1);
}
