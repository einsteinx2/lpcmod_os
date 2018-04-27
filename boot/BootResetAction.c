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
#include "FatFSAccessor.h"
#include "i2c.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "lib/LPCMod/BootLCD.h"
#include "xblast/settings/xblastSettingsImportExport.h"
#include "xblast/settings/xblastSettingsChangeTracker.h"
#include "cpu.h"
#include "config.h"
#include "video.h"
#include "memory_layout.h"
#include "lpcmod_v1.h"
#include "xblast/scriptEngine/xblastScriptEngine.h"
#include "xblast/settings/xblastSettings.h"
#include "cromwell.h"
#include "IconMenu.h"
#include "MenuActions.h"
#include "MenuInits.h"
#include "menu/misc/ConfirmDialog.h"
#include "XBlastScriptMenuActions.h"
#include "LEDMenuActions.h"
#include "FlashMenuActions.h"
#include "string.h"
#include "xblast/HardwareIdentifier.h"
#include "FlashDriver.h"
#include "lib/time/timeManagement.h"

JPEG jpegBackdrop;

int nTempCursorMbrX, nTempCursorMbrY;

extern volatile int nInteruptable;

volatile CURRENT_VIDEO_MODE_DETAILS vmode;

void ClearScreen (void)
{
    BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);
}

void printMainMenuHeader(void)
{
    //Length of array is set depending on how many revision can be uniquely identified.
    //Modify this enum if you modify the "XBOX_REVISION" enum in boot.h

    ClearScreen();

    VIDEO_CURSOR_POSX=(vmode.xmargin/*+64*/)*4;
    VIDEO_CURSOR_POSY=vmode.ymargin;

    printk("\n\n");
    if(isXBE())
    {
        printk("           \2"PROG_NAME" (XBE) v" VERSION "\n\n\2");
    }
    else
    {
        printk("           \2"PROG_NAME" (ROM) v" VERSION "\n\n\2");
    }

    VIDEO_ATTR=0xff00ff00;

    VIDEO_CURSOR_POSX=(vmode.xmargin/*+64*/)*4;
    VIDEO_CURSOR_POSY=vmode.ymargin+64;


    VIDEO_ATTR=0xff00ff00;
#ifdef DEV_FEATURES
    printk("           Modchip: %s    fHasHardware: 0x%04x   fSpecialEdition: %02x\n", getModchipName(), fHasHardware, fSpecialEdition);
    VIDEO_ATTR=0xffc8c8c8;
    const OBJECT_FLASH* bootFlash = NULL;
    Flash_ReadDeviceInfo(&bootFlash);
    printk("           THIS IS A WIP BUILD, flash manID= %x  devID= %x\n", bootFlash->flashType.m_bManufacturerId, bootFlash->flashType.m_bDeviceId);
#else
    printk("           Modchip: ");

    switch(fSpecialEdition)
    {
    case SYSCON_ID_V1_PRE_EDITION:
    	VIDEO_ATTR=0xffef37;
    	break;
    default:
        break;
    }
        printk("%s\n",getModchipName());
#endif
    VIDEO_ATTR=0xff00ff00;


   printk("           Xbox revision: %s ", getMotherboardRevisionString());
   if (xbox_ram > 64)
   {
        VIDEO_ATTR=0xff00ff00;
   }
   else
   {
        VIDEO_ATTR=0xffffa20f;
   }
   printk("  CPU: %uMHz   RAM: %dMiB\n", getCPUSPeedInMHz(), xbox_ram);

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

    if (I2CGetTemperature(&n, &nx))
    {
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

}

//////////////////////////////////////////////////////////////////////
//
//  BootResetAction()

extern void BootResetAction ( void )
{
    bool fMbrPresent=false;
    bool fFirstBoot=false;                    //Flag to indicate first boot since flash update
    int nTempCursorX, nTempCursorY;
    int n, nx, i, returnValue = 255;
    unsigned char tempFanSpeed = 20;
    int res, dcluster;
    _LPCmodSettings *tempLPCmodSettings;
    unsigned char* fileBuffPtr;;

    unsigned char EjectButtonPressed=0;

#ifdef SPITRACE
    //Required to populate GenPurposeIOs before toggling GPIOs.
    WriteToIO (XBLAST_CONTROL, FlashBank_OSBank);    // switch to proper bank
    LPCMod_WriteIO(0x4, 0x4); // /CS to '1'
#endif

    XBlastLogger(DEBUG_ALWAYS_SHOW, DBG_LVL_INFO, "XBlast OS is starting.");

    A19controlModBoot = BNKFULLTSOP;        //Start assuming no control over A19 line.

    //Set to NULL as it's not used yet.
    //gobalGenericPtr = NULL;
    
    xF70ELPCRegister = 0x03;       //Assume no control over the banks but we are booting from bank3
    x00FFLPCRegister = ReadFromIO(XODUS_CONTROL);       //Read A15 and D0 states.
                                                        //Should return 0x04 on normal boot, 0x08 on TSOP recovery.

    TSOPRecoveryMode = 0;
    //TSOPRecoveryMode = (x00FFLPCRegister & 0x08) >> 3;  //If we booted and A15 was already set.
                                                        //It means we are in TSOP recovery. Set to 1.
                                                        //We'll check later if TSOP flash is accessible.

#ifndef SPITRACE        //Do not reset GenPurposeIOs values as they've been updated when "LPCMod_WriteIO(0x4, 0x4)" function was called.
    GenPurposeIOs.GPO3 = 0;
    GenPurposeIOs.GPO2 = 0;
    GenPurposeIOs.GPO1 = 0;
    GenPurposeIOs.GPO0 = 0;
    GenPurposeIOs.GPI1 = 0;
    GenPurposeIOs.GPI0 = 0;
    GenPurposeIOs.A19BufEn = 0;
    GenPurposeIOs.EN_5V = 0;
#endif

    memcpy(&cromwell_config, (void*)(CODE_LOC_START + 0x20), sizeof(cromwell_config));
    memcpy(&cromwell_retryload, (void*)(CODE_LOC_START + 0x20 + sizeof(cromwell_config)), sizeof(cromwell_retryload));
    memcpy(&cromwell_2blversion, (void*)(CODE_LOC_START + 0x20 + sizeof(cromwell_config) + sizeof(cromwell_retryload)), sizeof(cromwell_2blversion));
    memcpy(&cromwell_2blsize, (void*)(CODE_LOC_START + 0x20 + sizeof(cromwell_config) + sizeof(cromwell_retryload) + sizeof(cromwell_2blversion)), sizeof(cromwell_2blsize));

    VIDEO_CURSOR_POSX=40;
    VIDEO_CURSOR_POSY=140;
        
    VIDEO_AV_MODE = 0xff;
    nInteruptable = 0;

    // prep our BIOS console print state
    VIDEO_ATTR = 0xffffffff;

    // init malloc() and free() structures
    MemoryManagementInitialization((void *)MEMORYMANAGERSTART, MEMORYMANAGERSIZE);
    XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_DEBUG, "Init soft MMU.");

    BootInterruptsWriteIdt();

    // initialize the PCI devices
    //bprintf("BOOT: starting PCI init\n\r");
    BootPciPeripheralInitialization();
    

    I2CTransmitWord(0x10, 0x1901); // no reset on eject
    if(I2CTransmitByteGetReturn(0x10, 0x03) & 0x01)
    {
        EjectButtonPressed = 1;
        I2CTransmitByteGetReturn(0x10, 0x11);       // dummy Query IRQ
        I2CWriteBytetoRegister(0x10, 0x03,0x00);    // Clear Tray Register
        I2CTransmitWord(0x10, 0x0c01); // close DVD tray
    }

    /* Here, the interrupts are Switched on now */
    BootPciInterruptEnable();
    /* We allow interrupts */
    nInteruptable = 1;
#ifndef SILENT_MODE
    printk("           BOOT: start USB init\n");
#endif

    BootStartUSB();
    XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "USB init done.");

    Flash_Init();

    identifyModchipHardware();

    // Reset the AGP bus and start with good condition
    BootAGPBUSInitialization();

    I2CTransmitByteGetReturn(0x10, 0x11);       // dummy Query IRQ
    I2CTransmitWord(0x10, 0x1a01); // Enable PIC interrupts. Cannot be deactivated once set.

    unsigned char readUSB = 0;
    if(EjectButtonPressed == 0 && isXBE() == false)
    {
        setLED("rrrr");       //Signal the user to press Eject button to avoid Quickboot.
    }
    wait_us_blocking(760000);

    XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Read persistent OS settings from flash.");
    if(bootReadXBlastOSSettings() == false)
    {
            XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_WARN, "No persistent OS settings found on flash. Created default settings.");
            fFirstBoot = true;
            LEDFirstBoot(NULL);
    }

#if 0
    /* We'll be doing it invariably berfore the 750ms delay instead...*/
    if(EjectButtonPressed == 0 && LPCmodSettings.OSsettings.Quickboot)
    {
        if(isXBE() == false)
        {
            setLED("rrrr");       //Signal the user to press Eject button to avoid Quickboot.
        }
    }
#endif


    if(isXBE() && isXBlastOnLPC() == false) //If coming from XBE and no XBlast Mod is detected
    {
        tempFanSpeed = I2CGetFanSpeed();
        if(tempFanSpeed < 10)
        {
            tempFanSpeed = 10;
        }
        else if(tempFanSpeed > 100)
        {
            tempFanSpeed = 100;
        }

        LPCmodSettings.OSsettings.fanSpeed = tempFanSpeed;      //Get previously set fan speed
    }
    else
    {
        // Make sure fan speed is always within normal values.
        if(LPCmodSettings.OSsettings.fanSpeed < 10)
        {
            LPCmodSettings.OSsettings.fanSpeed = 10;
        }
        else if(LPCmodSettings.OSsettings.fanSpeed > 100)
        {
            LPCmodSettings.OSsettings.fanSpeed = 100;
        }
        I2CSetFanSpeed(LPCmodSettings.OSsettings.fanSpeed);     //Else we're booting in ROM mode and have a fan speed to set.
    }
    XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_DEBUG, "Fan speed adjustment if needed.");

    if(isPureXBlast() && isXBlastOnTSOP())
    {
        //LPCmodSettings.OSsettings.TSOPcontrol = (ReadFromIO(XODUS_CONTROL) & 0x20) >> 5;     //A19ctrl maps to bit5
        LPCmodSettings.OSsettings.TSOPcontrol = (unsigned char)GenPurposeIOs.A19BufEn;
        XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Buffer enable for A19 control : %sabled.", GenPurposeIOs.A19BufEn? "En" : "Dis");
    }

    BootLCDInit();    //Basic init. Do it even if no LCD is connected on the system.
    XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "BootLCDInit done.");

    //Stuff to do right after loading persistent settings from flash.
    if(fFirstBoot == false)
    {
        if(emergencyRecoverSettings())
        {
                XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_WARN, "Emergency recover triggered. Resetting settings.");
                fFirstBoot = true;
                LEDFirstBoot(NULL);
        }

        if(isLCDSupported())
        {
            XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_DEBUG, "Check if we need to drive the LCD.");
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

    identifyXboxHardware();

    eepromChangeTrackerInit();
    BootEepromReadEntireEEPROM();
    memcpy(&origEeprom, &eeprom, sizeof(EEPROMDATA));
    XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Initial EEprom read.");
        
    I2CTransmitWord(0x10, 0x1b04); // unknown
        
    //Let's set that up right here.
    settingsTrackerInit();
    setCFGFileTransferPtr(&LPCmodSettings, &settingsPtrStruct);

    // Load and Init the Background image
    // clear the Video Ram
    memset((void *)FB_START,0x00,FB_SIZE);

    BootVgaInitializationKernelNG((CURRENT_VIDEO_MODE_DETAILS *)&vmode);
    jpegBackdrop.pData =NULL;
    jpegBackdrop.pBackdrop = NULL; //Static memory alloc now.


    if(isTSOPSplitCapable() == false)
    {
       LPCmodSettings.OSsettings.TSOPcontrol = 0;       //Make sure to not show split TSOP options. Useful if modchip was moved from 1 console to another.
    }

    //Load up some more custom settings right before booting to OS.
    if(fFirstBoot == false)
    {
        if(LPCmodSettings.OSsettings.runBootScript && isXBE() == false)
        {
            XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Running boot script.");
            if(LPCmodSettings.flashScript.scriptSize > 0)
            {
                i = BNKOS;
                runScript(LPCmodSettings.flashScript.scriptData, LPCmodSettings.flashScript.scriptSize, 1, &i);
            }
            XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Boot script execution done.");
        }

        if(isXBlastOnLPC() && isXBE() == false)       //Quickboot only if on the right hardware.
		{
            if(LPCmodSettings.OSsettings.Quickboot)
            {
                XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Check any Quickboot or EjectButton boot rule.");

                // No quickboot if both button pressed at that point.
                if(EjectButtonPressed == 0)
                {
                    if(traystate == ETS_NOTHING && LPCmodSettings.OSsettings.activeBank != BNKOS)
                    {
                        XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Going to Power Button Quickboot.");
                        quickboot(LPCmodSettings.OSsettings.activeBank);
                    }
                }
                else
                {
                    if(LPCmodSettings.OSsettings.altBank != BNKOS)
                    {
                        XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Eject button press boot detected.");
                        quickboot(LPCmodSettings.OSsettings.altBank);
                    }
                }
            }

            I2CTransmitByteGetReturn(0x10, 0x11);       // dummy Query IRQ
            I2CWriteBytetoRegister(0x10, 0x03,0x00);    // Clear Tray Register
            I2CTransmitWord(0x10, 0x0c01); // close DVD tray
        }

        XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_DEBUG, "No Quickboot or EjectButton boot this time.");
        initialSetLED(LPCmodSettings.OSsettings.LEDColor);
    }
    else
    {
        XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "First boot so no script or bank loading before going to OS at least once.");
    }

    if(BootVideoInitJPEGBackdropBuffer(&jpegBackdrop))
    { // decode and malloc backdrop bitmap
        extern int _start_backdrop;
        extern int _end_backdrop;
        BootVideoJpegUnpackAsRgb(
            (unsigned char *)&_start_backdrop,
             &jpegBackdrop,
        _end_backdrop - _start_backdrop
        );
    }
    // paint the backdrop
    XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Print Main Menu header.");
    printMainMenuHeader();

    // set Ethernet MAC address from EEPROM
    {
        volatile unsigned char * pb=(unsigned char *)0xfef000a8;  // Ethernet MMIO base + MAC register offset (<--thanks to Anders Gustafsson)
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
//    BootIdeWaitNotBusy(0x1f0);
//    wait_ms(100);
#ifndef SILENT_MODE
    printk("           Ready\n");
#endif


    XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Starting IDE init.");
    BootIdeInit();
    XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "IDE init done.");
    XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Starting FatFS init.");
    FatFS_init();
    XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "FatFS init done.");
    XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Starting DebugLogger init.");
    debugLoggerInit();
    XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "DebugLogger init done.");

    //Load settings from xblast.cfg file if no settings were detected.
    //But first do we have a HDD on Master?
    if(tsaHarddiskInfo[0].m_fDriveExists && tsaHarddiskInfo[0].m_fAtapi == false)
    {
        XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_DEBUG, "Master HDD exist.");
        if(fFirstBoot == false)
        {
            //TODO: Load optional JPEG backdrop from HDD here. Maybe fetch skin name from cfg file?
            XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Trying to load JPEGs from HDD.");
            if(LPCMod_ReadJPGFromHDD("MASTER_C:"PathSep"XBlast"PathSep"icons.jpg") == false)
            {
                XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "\"icons.jpg\" loaded. Moving on to \"backdrop.jpg\".");
            }
            if(LPCMod_ReadJPGFromHDD("MASTER_C:"PathSep"XBlast"PathSep"backdrop.jpg") == false)
            {
                XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "\"backdrop.jpg\" loaded. Repainting.");
                printMainMenuHeader();
            }

            if(isXBE() && isXBlastOnLPC() == false)
            {
                XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Trying to load settings from cfg file on HDD.");
                _LPCmodSettings tempLPCmodSettings;
                returnValue = LPCMod_ReadCFGFromHDD(&tempLPCmodSettings, &settingsPtrStruct);
                if(returnValue == 0)
                {
                    importNewSettingsFromCFGLoad(&tempLPCmodSettings);
                    res = 0;
                    FILEX fileHandle = fatxopen("MASTER_C:"PathSep"XBlast"PathSep"scripts"PathSep"bank.script", FileOpenMode_OpenExistingOnly | FileOpenMode_Read);
                    if(fileHandle)
                    {
                        if(fatxsize(fileHandle) > 0)
                        {
                            res = 1;
                        }
                        fatxclose(fileHandle);
                    }
                    if(0 == res)
                    {
                        XBlastLogger(DEBUG_SETTINGS, DBG_LVL_DEBUG, "Could not find valid bank.script file on HDD. Forcing setting to '0'.");
                        LPCmodSettings.OSsettings.runBankScript = 0;
                    }

                    res = 0;
                     fileHandle = fatxopen("MASTER_C:"PathSep"XBlast"PathSep"scripts"PathSep"boot.script", FileOpenMode_OpenExistingOnly | FileOpenMode_Read);
                     if(fileHandle)
                     {
                         if(fatxsize(fileHandle) > 0)
                         {
                             res = 1;
                         }
                         fatxclose(fileHandle);
                     }
                     if(0 == res)
                     {
                         XBlastLogger(DEBUG_SETTINGS, DBG_LVL_DEBUG, "Could not find valid boot.script file on HDD. Forcing setting to '0'.");
                         LPCmodSettings.OSsettings.runBootScript = 0;
                     }

                    //bootScriptSize should not have changed if we're here.
                    if(LPCmodSettings.OSsettings.runBootScript && LPCmodSettings.flashScript.scriptSize == 0)
                    {
                        XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Running boot script.");
                        i = BNKOS;
                        loadRunScriptWithParams("MASTER_C:"PathSep"XBlast"PathSep"scripts"PathSep"boot.script", 1, &i);

                        XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Boot script execution done.");
                    }
                }
            }
        }
    }

    VIDEO_CURSOR_POSX=nTempCursorX;
    VIDEO_CURSOR_POSY=nTempCursorY;
    VIDEO_CURSOR_POSX=vmode.xmargin;
    VIDEO_CURSOR_POSY=vmode.ymargin;

    printk("\n\n\n\n");

    nTempCursorMbrX=VIDEO_CURSOR_POSX;
    nTempCursorMbrY=VIDEO_CURSOR_POSY;

    videosavepage = malloc(FB_SIZE);

    //Check for unformatted drives.
    formatNewDrives();
    

//    printk("i2C=%d SMC=%d, IDE=%d, tick=%d una=%d unb=%d\n", nCountI2cinterrupts, nCountInterruptsSmc, nCountInterruptsIde, BIOS_TICK_COUNT, nCountUnusedInterrupts, nCountUnusedInterruptsPic2);
    IconMenuInit();
    XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Starting IconMenu.");
    while(IconMenu())
    {
        ClearScreen();
        printMainMenuHeader();
    }
    //Good practice.
    free(videosavepage);

    //Should never come back here.
    while(1);
}
