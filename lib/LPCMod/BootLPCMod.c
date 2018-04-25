/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "FatFSAccessor.h"
#include "video.h"
#include "BootLPCMod.h"
#include "lpcmod_v1.h"
#include "LEDMenuActions.h"
#include "xblast/HardwareIdentifier.h"
#include "string.h"
#include "stdlib.h"
#include "i2c.h"
#include "boot.h"
#include "stdio.h"
#include "BootIde.h"
#include "MenuActions.h"
#include "misc/ConfirmDialog.h"
#include "Gentoox.h"
#include "lib/cromwell/cromString.h"

//Probes CPLD for chip revision and return a single byte ID.
//SmartXX compliant but need to mask out upper nibble
unsigned short LPCMod_HW_rev(void){
	unsigned short returnValue = ReadFromIO(SYSCON_REG);

    return returnValue;
}

void LPCMod_ReadIO(struct _GenPurposeIOs *GPIOstruct){
    struct _GenPurposeIOs *localGPIOstruct;
    unsigned char temp;

    //We have a XBlast Mod detected or else there's a strong possibility function will return 0xff;
    if(fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_V1_TSOP)
    {
        temp = ReadFromIO(XBLAST_IO);
    }
    else
    {
        temp = 0;
    }

    //If no valid pointer is specified, take Global struct.
    if(GPIOstruct == NULL)
        localGPIOstruct = &GenPurposeIOs;
    else
        localGPIOstruct = GPIOstruct;

    localGPIOstruct->GPO3 = (temp & 0x80) >> 7;
    localGPIOstruct->GPO2 = (temp & 0x40) >> 6;
    localGPIOstruct->GPO1 = (temp & 0x20) >> 5;
    localGPIOstruct->GPO0 = (temp & 0x10) >> 4;
    localGPIOstruct->GPI1 = (temp & 0x08) >> 3;
    localGPIOstruct->GPI0 = (temp & 0x04) >> 2;
    localGPIOstruct->A19BufEn = (temp & 0x02) >> 1;
    localGPIOstruct->EN_5V = (temp & 0x01);
}

void LPCMod_WriteIO(unsigned char port, unsigned char value){
    unsigned char temp;

    //We have a XBlast Mod detected or else there's a strong possibility function will return 0xff;
    if(fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_V1_TSOP)
        temp = ReadFromIO(XBLAST_IO);
    else
        temp = 0;

    GenPurposeIOs.GPO3 = (port & 0x08)? (value & 0x08) >> 3: (temp & 0x80) >> 7;
    GenPurposeIOs.GPO2 = (port & 0x04)? (value & 0x04) >> 2: (temp & 0x40) >> 6;
    GenPurposeIOs.GPO1 = (port & 0x02)? (value & 0x02) >> 1 : (temp & 0x20) >> 5;
    GenPurposeIOs.GPO0 = (port & 0x01)? (value & 0x01) : (temp & 0x10) >> 4;
    GenPurposeIOs.GPI1 = (temp & 0x08) >> 3;
    GenPurposeIOs.GPI0 = (temp & 0x04) >> 2;
    GenPurposeIOs.A19BufEn = (temp & 0x02) >> 1;
    GenPurposeIOs.EN_5V = (temp & 0x01);

    LPCMod_WriteGenPurposeIOs();
}

void LPCMod_FastWriteIO(unsigned char port, unsigned char value){
    GenPurposeIOs.GPO3 = (port & 0x08)? (value & 0x08) >> 3: GenPurposeIOs.GPO3;
    GenPurposeIOs.GPO2 = (port & 0x04)? (value & 0x04) >> 2: GenPurposeIOs.GPO2;
    GenPurposeIOs.GPO1 = (port & 0x02)? (value & 0x02) >> 1: GenPurposeIOs.GPO1;
    GenPurposeIOs.GPO0 = (port & 0x01)? (value & 0x01) : GenPurposeIOs.GPO0;

    LPCMod_WriteGenPurposeIOs();
}

void LPCMod_WriteGenPurposeIOs(void)
{
    WriteToIO(XBLAST_IO, (GenPurposeIOs.GPO3 << 7) | (GenPurposeIOs.GPO2 << 6) | (GenPurposeIOs.GPO1 << 5) | (GenPurposeIOs.GPO0 << 4) | GenPurposeIOs.EN_5V);
}

void quickboot(unsigned char bank)
{
    if(bank > BOOTFROMTSOP)
    {
        XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Booting XBlast flash bank");
        switchBootBank(bank);
    }
    else
    {
        XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Booting TSOP flash bank");
        //If booting from TSOP, use of the XODUS_CONTROL register is fine.
        if(getMotherboardRevision() == XboxMotherboardRevision_1_6 || getMotherboardRevision() == XboxMotherboardRevision_UNKNOWN)
        {
            switchBootBank(KILL_MOD);    // switch to original bios. Mute modchip.
        }
        else
        {
            switchBootBank(bank);    // switch to original bios but modchip listen to LPC commands.
                                                                     // Lock flash bank control with OSBNKCTRLBIT.
        }
    }
    I2CTransmitWord(0x10, 0x1b00 + ( I2CTransmitByteGetReturn(0x10, 0x1b) & 0xfb )); // clear noani-bit
    BootStopUSB();
    I2CRebootQuick();
    while(1);
}

int LPCMod_ReadJPGFromHDD(const char *jpgFilename)
{
    unsigned char* fileBuff;
    
    FILEX handle = fatxopen(jpgFilename, FileOpenMode_OpenExistingOnly | FileOpenMode_Read);

    if(0 == handle)
    {
        XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "No jpg file.");
        return -1;
    }

    unsigned int size = fatxsize(handle);
    fileBuff = malloc(size * sizeof(unsigned char));

    if(NULL == fileBuff)
    {
        XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_FATAL, "malloc failed.");
        return -1;
    }

    if(fatxread(handle, fileBuff, size) != size)
    {
        free(fileBuff);
        XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_ERROR, "Read incomplete.");
        return -1;
    }

    BootVideoJpegUnpackAsRgb(fileBuff, &jpegBackdrop, size);
    free(fileBuff);

    if(fatxclose(handle))
    {
        XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_ERROR, "Error close jpg file.");
    }

    return 0;
}

void formatNewDrives(void)
{
    const char* failureString = "\n           Could not format ";
    unsigned char i;
    memcpy(videosavepage,(void*)FB_START,FB_SIZE);

    for (i = 0; i < NbDrivesSupported; ++i)
    {
        if(BootIdeDeviceConnected(i) && 0 == BootIdeDeviceIsATAPI(i) && XBOX_EXTEND_STARTLBA <= BootIdeGetSectorCount(i) &&  0 == BootIdeDeviceIsLocked(i) && 0 == isFATXFormattedDrive(i))
        {
            XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "No FATX detected on %s HDD.", i ? "Slave" : "Master");
            char ConfirmDialogString[50];
            sprintf(ConfirmDialogString, "Format new drive (%s)?", i ? "slave":"master");
            if(ConfirmDialog(ConfirmDialogString, 1) == false)
            {
                XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "Formatting base partitions.");
                fdisk(i, XboxDiskLayout_Base);
                if(fatxmkfs(i, Part_C))
                {
                    cromwellError();
                    printk("%sC:\\", failureString);
                    break;
                }
                if(fatxmkfs(i, Part_E))
                {
                    cromwellError();
                    printk("%sE:\\", failureString);
                    break;
                }
                if(fatxmkfs(i, Part_X))
                {
                    cromwellError();
                    printk("%sX:\\", failureString);
                    break;
                }
                if(fatxmkfs(i, Part_Y))
                {
                    cromwellError();
                    printk("%sY:\\", failureString);
                    break;
                }
                if(fatxmkfs(i, Part_Z))
                {
                    cromwellError();
                    printk("%sZ:\\", failureString);
                    break;
                }

                if((XBOX_EXTEND_STARTLBA + SYSTEM_LBASIZE) <= BootIdeGetSectorCount(i))
                {
                    XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_DEBUG, "Show user extended partitions format options.");
                    DrawLargeHDDTextMenu(i);//Launch LargeHDDMenuInit textmenu.
                }
                XBlastLogger(DEBUG_BOOT_LOG, DBG_LVL_INFO, "HDD format done.");
            }
        }
    }
    memcpy((void*)FB_START,videosavepage,FB_SIZE);
}

//Use this function only for in OS operations.
void switchOSBank(FlashBank bank) {
    //Only send command if XBlast compatible device is found
    if(isXBlastOnLPC())
    {
        currentFlashBank = bank;
        xF70ELPCRegister = bank;
        WriteToIO (XBLAST_CONTROL, bank);    // switch to proper bank
                                             //Send OSBNKCTRLBIT when toggling a bank other than BNKOS.
    }
}

//Use this function only when you're about to boot into another bank.
void switchBootBank(FlashBank bank)
{
    //Only send command if XBlast compatible device is found
    if(isXBlastCompatible())
    {
        unsigned char resultBank = bank;
        //currentFlashBank = NOBNKID;         //We won't be coming back from this!
        if(bank > BOOTFROMTSOP)       //We're asked to boot from XBlast's flash
        {
            resultBank |= A19controlModBoot;  //Apply custom A19 control (if need be).
        }
        WriteToIO (XBLAST_CONTROL, resultBank); // switch to proper bank from booting register
    }
}

void WriteToIO(unsigned short _port, unsigned char _data)
{
   __asm__ ("out %%al, %%dx" : : "a" (_data), "d" (_port));
}

unsigned char ReadFromIO(unsigned short address)
{
   unsigned char data;
   __asm__ __volatile__ ("inb %w1,%0":"=a" (data):"Nd" (address));
   return data;
}

#ifdef SPITRACE
void printTextSPI(const char* buffer)
{
    char i;

    LPCMod_FastWriteIO(0x2, 0); //CLK to '0'


    //Will NOT send null terminating character at the end.
    while('\0' != *buffer)
    {
        LPCMod_FastWriteIO(0x4, 0); // /CS to '0'
        for(i = 7; i >= 0; i--){
            LPCMod_FastWriteIO(0x3, (*buffer >> i)&0x01); //CLK to '0' + MOSI data bit set
            LPCMod_FastWriteIO(0x2, 0x2); //CLK to '1'
        }
        LPCMod_FastWriteIO(0x2, 0); //CLK to '0'.
        LPCMod_FastWriteIO(0x4, 0x4); // /CS to '1'
        buffer++;
    }
}

#endif

