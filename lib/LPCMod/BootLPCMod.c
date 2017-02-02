/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "BootFATX.h"
#include "video.h"
#include "BootLPCMod.h"
#include "lpcmod_v1.h"
#include "LEDMenuActions.h"
#include "lib/time/timeManagement.h"
#include "xblast/HardwareIdentifier.h"
#include "string.h"
#include "stdlib.h"
#include "i2c.h"
#include "boot.h"
#include "stdio.h"

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
        debugSPIPrint("Booting XBlast flash bank\n");
        switchBootBank(bank);
    }
    else
    {
        debugSPIPrint("Booting TSOP flash bank\n");
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
    FATXFILEINFO fileinfo;
    FATXPartition *partition;
    int res = false;
    int dcluster;
    

    partition = OpenFATXPartition(0, SECTOR_SYSTEM, SYSTEM_SIZE);
    if(partition != NULL){
        dcluster = FATXFindDir(partition, FATX_ROOT_FAT_CLUSTER, "XBlast");
        if((dcluster != -1) && (dcluster != 1)) {
            res = FATXFindFile(partition, (char *)jpgFilename, FATX_ROOT_FAT_CLUSTER, &fileinfo);
        }
        if(LoadFATXFile(partition, (char *)jpgFilename, &fileinfo)){
		if(res && fileinfo.fileSize){        //File exist and is loaded.
		    BootVideoJpegUnpackAsRgb(fileinfo.buffer, &jpegBackdrop, fileinfo.fileSize);
		    free(fileinfo.buffer);
		}
		else{
		    return -1;
		}
	}
	else
	    return -1;
        CloseFATXPartition(partition);
    }
    else
        return -1;

    return 0;
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
void printTextSPI(const char * functionName, char * buffer, ...)
{
    unsigned char pos;
    char i;
    int stringLength;
    char tempBuf[200];
    char outputBuf[200];

    va_list args;
    LPCMod_FastWriteIO(0x2, 0); //CLK to '0'
    if(buffer != NULL){
        va_start(args, buffer);
        vsprintf(tempBuf,buffer,args);
        sprintf(outputBuf, "[%s] %s", functionName, tempBuf);
    }
    else{
        sprintf(outputBuf, "[%s]\n", functionName);
    }

    stringLength = strlen(outputBuf);
    if(stringLength > 200)
        stringLength = 200;

    //Will NOT send null terminating character at the end.
    for(pos = 0; pos < stringLength; pos++){
        LPCMod_FastWriteIO(0x4, 0); // /CS to '0'
        for(i = 7; i >= 0; i--){
            LPCMod_FastWriteIO(0x3, (outputBuf[pos] >> i)&0x01); //CLK to '0' + MOSI data bit set
            LPCMod_FastWriteIO(0x2, 0x2); //CLK to '1'
        }
        LPCMod_FastWriteIO(0x2, 0); //CLK to '0'.
        LPCMod_FastWriteIO(0x4, 0x4); // /CS to '1'
    }
    //If you miss characters, add delay function here (wait_us()). A couple microseconds should give enough time for the Arduino to catchup.
    wait_us_blocking(50);
}
#endif

