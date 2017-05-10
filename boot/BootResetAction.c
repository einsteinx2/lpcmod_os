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
#include "BootFATX.h"
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
    FATXPartition *partition;
    FATXFILEINFO fileinfo;

    unsigned char EjectButtonPressed=0;

#ifdef SPITRACE
    //Required to populate GenPurposeIOs before toggling GPIOs.
    LPCMod_WriteIO(0x4, 0x4); // /CS to '1'
    switchOSBank(FlashBank_OSBank);
#endif

    debugSPIPrint("XBlast OS is starting.\n");

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
    debugSPIPrint("Init soft MMU.\n");

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
    debugSPIPrint("USB init done.\n");

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

    debugSPIPrint("Read persistent OS settings from flash.\n");
    if(bootReadXBlastOSSettings() == false)
    {
            debugSPIPrint("No persistent OS settings found on flash. Created default settings.\n");
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
    debugSPIPrint("Fan speed adjustment if needed.\n");

    if(isPureXBlast() && isXBlastOnTSOP())
    {
        //LPCmodSettings.OSsettings.TSOPcontrol = (ReadFromIO(XODUS_CONTROL) & 0x20) >> 5;     //A19ctrl maps to bit5
        LPCmodSettings.OSsettings.TSOPcontrol = (unsigned char)GenPurposeIOs.A19BufEn;
        debugSPIPrint("Buffer enable for A19 control : %sabled.\n", GenPurposeIOs.A19BufEn? "En" : "Dis");
    }

    BootLCDInit();    //Basic init. Do it even if no LCD is connected on the system.
    debugSPIPrint("BootLCDInit done.\n");

    //Stuff to do right after loading persistent settings from flash.
    if(fFirstBoot == false)
    {
        if(emergencyRecoverSettings())
        {
                debugSPIPrint("Emergency recover triggered. Resetting settings.\n");
                fFirstBoot = true;
                LEDFirstBoot(NULL);
        }

        if(isLCDSupported())
        {
            debugSPIPrint("Check if we need to drive the LCD.\n");
            assertInitLCD();                            //Function in charge of checking if a init of LCD is needed.
            debugSPIPrint("assertInitLCD done.\n");
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
    debugSPIPrint("EEprom read.\n");
        
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
            debugSPIPrint("Running boot script.\n");
            if(LPCmodSettings.flashScript.scriptSize > 0)
            {
                i = BNKOS;
                runScript(LPCmodSettings.flashScript.scriptData, LPCmodSettings.flashScript.scriptSize, 1, &i);
            }
            debugSPIPrint("Boot script execution done.\n");
        }

        if(isXBlastOnLPC() && isXBE() == false)       //Quickboot only if on the right hardware.
		{
            if(LPCmodSettings.OSsettings.Quickboot)
            {
                debugSPIPrint("Check any Quickboot or EjectButton boot rule.\n");

                // No quickboot if both button pressed at that point.
                if(EjectButtonPressed == 0)
                {
                    if(traystate == ETS_NOTHING && LPCmodSettings.OSsettings.activeBank != BNKOS)
                    {
                        debugSPIPrint("Going to Quickboot.\n");
                        quickboot(LPCmodSettings.OSsettings.activeBank);
                    }
                }
                else
                {
                    if(LPCmodSettings.OSsettings.altBank != BNKOS)
                    {
                        debugSPIPrint("Eject button press boot detected.\n");
                        debugSPIPrint("Going to alt Quickboot.\n");
                        quickboot(LPCmodSettings.OSsettings.altBank);
                    }
                }
            }

            I2CTransmitByteGetReturn(0x10, 0x11);       // dummy Query IRQ
            I2CWriteBytetoRegister(0x10, 0x03,0x00);    // Clear Tray Register
            I2CTransmitWord(0x10, 0x0c01); // close DVD tray
        }

        debugSPIPrint("No Quickboot or EjectButton boot this time.\n");
        initialSetLED(LPCmodSettings.OSsettings.LEDColor);
    }
    else
    {
        debugSPIPrint("First boot so no script or bank loading before going to OS at least once.\n");
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
    debugSPIPrint("Print Main Menu header.\n");
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


    debugSPIPrint("Starting IDE init.\n");
    BootIdeInit();
    debugSPIPrint("IDE init done.\n");

    //Load settings from xblast.cfg file if no settings were detected.
    //But first do we have a HDD on Master?
    if(tsaHarddiskInfo[0].m_fDriveExists && tsaHarddiskInfo[0].m_fAtapi == false)
    {
        debugSPIPrint("Master HDD exist.\n");
        if(fFirstBoot == false)
        {
            //TODO: Load optional JPEG backdrop from HDD here. Maybe fetch skin name from cfg file?
            debugSPIPrint("Trying to load new JPEG from HDD.\n");
            if(LPCMod_ReadJPGFromHDD("\\XBlast\\icons.jpg") == false)
            {
                debugSPIPrint("\"Ã¬cons.jpg\" loaded. Moving on to \"backdrop.jpg\".\n");
            }
            if(LPCMod_ReadJPGFromHDD("\\XBlast\\backdrop.jpg") == false)
            {
                debugSPIPrint("\"backdrop.jpg\" loaded. Repainting.\n");
                printMainMenuHeader();
            }

            if(isXBE() && isXBlastOnLPC() == false)
            {
                debugSPIPrint("Trying to load settings from cfg file on HDD.\n");
                _LPCmodSettings tempLPCmodSettings;
                returnValue = LPCMod_ReadCFGFromHDD(&tempLPCmodSettings, &settingsPtrStruct);
                if(returnValue == 0)
                {
                    importNewSettingsFromCFGLoad(&tempLPCmodSettings);

                    partition = OpenFATXPartition(0, SECTOR_SYSTEM, SYSTEM_SIZE);
                    if(partition != NULL)
                    {
                        dcluster = FATXFindDir(partition, FATX_ROOT_FAT_CLUSTER, "XBlast");
                        if((dcluster != -1) && (dcluster != 1))
                        {
                            dcluster = FATXFindDir(partition, dcluster, "scripts");
                        }
                        if((dcluster != -1) && (dcluster != 1))
                        {
                            res = FATXFindFile(partition, "bank.script", FATX_ROOT_FAT_CLUSTER, &fileinfo);
                            if(res == 0 || fileinfo.fileSize == 0)
                            {
                                LPCmodSettings.OSsettings.runBankScript = 0;
                            }
                            res = FATXFindFile(partition, "boot.script", FATX_ROOT_FAT_CLUSTER, &fileinfo);
                            if(res == 0 || fileinfo.fileSize == 0)
                            {
                                LPCmodSettings.OSsettings.runBootScript = 0;
                            }
                        }
                            CloseFATXPartition(partition);
                    }
                    //bootScriptSize should not have changed if we're here.
                    if(LPCmodSettings.OSsettings.runBootScript && LPCmodSettings.flashScript.scriptSize == 0)
                    {
                        debugSPIPrint("Running boot script.\n");
                        if(loadScriptFromHDD("\\XBlast\\scripts\\boot.script", &fileinfo))
                        {
                            i = BNKOS;
                            runScript(fileinfo.buffer, fileinfo.fileSize, 1, &i);
                        }
                        debugSPIPrint("Boot script execution done.\n");
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
    for (i=0; i<2; ++i)
    {
        if (tsaHarddiskInfo[i].m_fDriveExists && tsaHarddiskInfo[i].m_fAtapi == false
            && tsaHarddiskInfo[i].m_dwCountSectorsTotal >= (SECTOR_EXTEND - 1)
            && (tsaHarddiskInfo[i].m_securitySettings&0x0002) == 0)
        {    //Drive not locked.
            if(tsaHarddiskInfo[i].m_enumDriveType != EDT_XBOXFS)
            {
                debugSPIPrint("No FATX detected on %s HDD.\n", i ? "Slave" : "Master");
                // We save the complete framebuffer to memory (we restore at exit)
                //videosavepage = malloc(FB_SIZE);
                memcpy(videosavepage,(void*)FB_START,FB_SIZE);
                char ConfirmDialogString[50];
                sprintf(ConfirmDialogString, "Format new drive (%s)?", i ? "slave":"master");
                if(ConfirmDialog(ConfirmDialogString, 1) == false)
                {
                    debugSPIPrint("Formatting base partitions.\n");
                    FATXFormatDriveC(i, 0);                     //'0' is for non verbose
                    FATXFormatDriveE(i, 0);
                    FATXFormatCacheDrives(i, 0);
                    FATXSetBRFR(i);
                    //If there's enough sectors to make F and/or G drive(s).
                    if(tsaHarddiskInfo[i].m_dwCountSectorsTotal >= (SECTOR_EXTEND + SECTORS_SYSTEM))
                    {
                        debugSPIPrint("Show user extended partitions format options.\n");
                        DrawLargeHDDTextMenu(i);//Launch LargeHDDMenuInit textmenu.
                    }

                    if(tsaHarddiskInfo[i].m_fHasMbr == 0)       //No MBR
                    {
                        FATXSetInitMBR(i); // Since I'm such a nice program, I will integrate the partition table to the MBR.
                    }
                    debugSPIPrint("HDD format done.\n");
                }
                memcpy((void*)FB_START,videosavepage,FB_SIZE);
                //free(videosavepage);
            }
        }
    }
    
    
//    printk("i2C=%d SMC=%d, IDE=%d, tick=%d una=%d unb=%d\n", nCountI2cinterrupts, nCountInterruptsSmc, nCountInterruptsIde, BIOS_TICK_COUNT, nCountUnusedInterrupts, nCountUnusedInterruptsPic2);
    IconMenuInit();
    debugSPIPrint("Starting IconMenu.\n");
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
