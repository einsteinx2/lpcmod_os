/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 /* 2003-01-07  andy@warmcat.com  Created
 */

#include "boot.h"
#include "BootFlash.h"
#include "memory_layout.h"
#include "video.h"
#include "include/lpcmod_v1.h"

//Selects which function should be called for flashing.
bool FlashFileFromBuffer(u8 *fileBuf, u32 fileSize, bool askConfirm){
    int res;
    char * stringTemp;
    int offset = 0;
    if (fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_XT ||
        (LPCmodSettings.OSsettings.TSOPcontrol && fHasHardware == SYSCON_ID_V1_TSOP)) {
        if (currentFlashBank == BNKOS) {
            if (!askConfirm || !ConfirmDialog ("               Confirm update XBlast OS?", 1)) {
                res = BootReflashAndReset (fileBuf, offset, fileSize);
            }
            else
                res = -1;
        }
        else {
            if (currentFlashBank == BNK512)
                stringTemp = "             Confirm flash bank0(512KB)?";
            else if (currentFlashBank == BNK256)
                stringTemp = "             Confirm flash bank1(256KB)?";
            else if (currentFlashBank == BNKTSOPSPLIT0)
                stringTemp = "             Confirm TSOP bank0(512KB)?";
            else if (currentFlashBank == BNKTSOPSPLIT1)
                stringTemp = "             Confirm TSOP bank1(512KB)?";
            //The 2 cases below shouldn't happen.
            else if (currentFlashBank == BNKFULLTSOP)
                stringTemp = "                Confirm TSOP (whole)?";
            else
                stringTemp = "                 Confirm flash bank?";
            if (!askConfirm || !ConfirmDialog (stringTemp, 1)) {
                res = BootReflash(fileBuf, offset, fileSize);
            }
            else
                res = -1;
        }
    }
    else { //If no XBlast mod or XBlast detected, from full(not split) TSOP.
        if (!askConfirm || !ConfirmDialog ("               Confirm flash active bank?", 1)) {
            res = BootReflashAndReset(fileBuf, offset, fileSize);
        }
        else
            res = -1;
    }

    return BootFlashPrintResult(res, fileSize);
}


 // callback to show progress
bool BootFlashUserInterface(void * pvoidObjectFlash, ENUM_EVENTS ee, u32 dwPos, u32 dwExtent) {
    if(ee==EE_ERASE_UPDATE){
        DisplayProgressBar(dwPos,dwExtent,0xffffff00);
    }
    else if(ee==EE_PROGRAM_UPDATE){
        DisplayProgressBar(dwPos,dwExtent,0xff00ff00);
    }
    else if(ee==EE_VERIFY_UPDATE){
         DisplayProgressBar(dwPos,dwExtent,0xffff00ff);
    }
    return true;
}

int BootReflashAndReset(u8 *pbNewData, u32 dwStartOffset, u32 dwLength)
{
    OBJECT_FLASH of;
    bool fMore=true;

    // A bit hacky, but easier to maintain.
    const KNOWN_FLASH_TYPE aknownflashtypesDefault[] = {
        #include "flashtypes.h"
    };

    // prep our flash object with start address and params
    of.m_pbMemoryMappedStartAddress=(u8 *)LPCFlashadress;
    of.m_dwStartOffset=dwStartOffset;           //Must always be 0!
    of.m_dwLengthUsedArea=dwLength;
    of.m_pcallbackFlash=BootFlashUserInterface;
/*
    if(fHasHardware == SYSCON_ID_XX1 ||
       fHasHardware == SYSCON_ID_XX2 ||
       fHasHardware == SYSCON_ID_XXOPX ||
       fHasHardware == SYSCON_ID_XX3){
        IoOutputByte(SMARTXX_FLASH_WRITEPROTECT , 1);       //Enable flash write on SmartXX mods.
    }
*/
    // check device type and parameters are sane
    if(!BootFlashGetDescriptor(&of, (KNOWN_FLASH_TYPE *)&aknownflashtypesDefault[0]))
        return 1; // unable to ID device - fail
    if(!of.m_fIsBelievedCapableOfWriteAndErase)
        return 2; // seems to be write-protected - fail
    if((of.m_dwLengthInBytes<(dwStartOffset+dwLength)) ||
       ((fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_XT) && of.m_dwLengthUsedArea != 262144) ||
        (dwLength % 262144) != 0 || dwLength == 0)                       //Image size is not a multiple of 256KB or is 0.
        return 3; // requested layout won't fit device - sanity check fail
    if((fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_XT) && currentFlashBank == BNKOS){ //Only check when on a XBlast mod. For the rest, I don't care.
        if(assertOSUpdateValidInput(pbNewData))
            return 4;  //Not valid XBlast OS image.
        if(crc32buf(pbNewData,0x3F000) != *(u32 *)&pbNewData[0x3FDFC])
            return 5;
    }
    else{       //If not XBlast Mod on BNKOS, mirror image to fill the entire size of detected flash up to 1MB
        mirrorImage(pbNewData, dwLength, &of);
    }
    
    // committed to reflash now
    while(fMore) {
        VIDEO_ATTR=0xffef37;
        printk("\n\n\n\n\n\n\n\n\n           \2%s\n\2\n", (fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_XT)?"Updating XBlast OS...":"Updating flash bank...");
        VIDEO_ATTR=0xffffff;
        printk("           WARNING!\n"
                 "           Do not turn off your console during this process!\n"
                 "           Your console should automatically reboot when this\n"
                 "           is done.  However, if it does not, please manually\n"
                 "           do so by pressing the power button once the LED has\n"
                 "           turned flashing amber (oxox)\n\n\n"
                 "           Flash chip: %s\n"
                 "           ManID=0x%02x , DevID=0x%02x", of.m_szFlashDescription, of.m_bManufacturerId, of.m_bDeviceId);

        if(BootFlashEraseMinimalRegion(&of)) {
            if(BootFlashProgram(&of, pbNewData) > 0) {
                fMore=false;  // good situation

                // Set LED to oxox.
                inputLED();

                I2CRebootSlow();
                while(1);

            }
            else { // failed program
                //printk("           Programming failed...\n");
/*
                if(fHasHardware == SYSCON_ID_XX1 ||
                   fHasHardware == SYSCON_ID_XX2 ||
                   fHasHardware == SYSCON_ID_XXOPX ||
                   fHasHardware == SYSCON_ID_XX3){
                    IoOutputByte(SMARTXX_FLASH_WRITEPROTECT , 0);       //Disable flash write on SmartXX mods.
                }
*/
                return -3;
            }
        }
        else { // failed erase
            //printk("           Erasing failed...\n");
/*
            if(fHasHardware == SYSCON_ID_XX1 ||
               fHasHardware == SYSCON_ID_XX2 ||
               fHasHardware == SYSCON_ID_XXOPX ||
               fHasHardware == SYSCON_ID_XX3){
                IoOutputByte(SMARTXX_FLASH_WRITEPROTECT , 0);       //Disable flash write on SmartXX mods.
            }
*/
            return -2;
        }
    }
    return 0; // keep compiler happy
}

/*Will only be used when XBlast Mod is detected*/
int BootReflash(u8 *pbNewData, u32 dwStartOffset, u32 dwLength)
{
    OBJECT_FLASH of;
    bool fMore=true;

    // A bit hacky, but easier to maintain.
    const KNOWN_FLASH_TYPE aknownflashtypesDefault[] = {
        #include "flashtypes.h"
    };

    // prep our flash object with start address and params
    of.m_pbMemoryMappedStartAddress=(u8 *)LPCFlashadress;
    of.m_dwStartOffset=dwStartOffset;
    of.m_dwLengthUsedArea=dwLength;
    of.m_pcallbackFlash=BootFlashUserInterface;

    // check device type and parameters are sane
    if(!BootFlashGetDescriptor(&of, (KNOWN_FLASH_TYPE *)&aknownflashtypesDefault[0]))
        return 1; // unable to ID device - fail
    if(!of.m_fIsBelievedCapableOfWriteAndErase)
        return 2; // seems to be write-protected - fail
    //512KB image mirror if necessary. BNK512 and split TSOP make use of this. Other cases are treated in the "else" portion.
    if(currentFlashBank == BNK512 || (fHasHardware == SYSCON_ID_V1_TSOP && LPCmodSettings.OSsettings.TSOPcontrol)){
        if(dwLength != 524288){             //If input image is not 512KB
            if(dwLength == 262144){         //If a 256KB image is to be flashed.
                memcpy(&pbNewData[262144], &pbNewData[0], 262144);  //Mirror image in the next 256KB segment of the 1MB buffer.
                dwLength = 262144 * 2;      //Image to flash is now 512KB
                of.m_dwLengthUsedArea=dwLength;
            }
            else
                return 3;   //Wrong file size.
        }
    }
    else if(currentFlashBank == BNK256){
        if(dwLength != 262144)
            return 3;
    }
    //Current bank is NOT BNK512, BNK256 or a split TSOP bank.
    else{  //Highly improbable since this function will only be called when XBlast Mod's is detected or TSOP is split and these cases are covered above.
        if(dwLength > 0 && (dwLength % 262144) != 0) //Image is a multiple of 256KB and not 0.
            mirrorImage(pbNewData, dwLength, &of);
        else
            return 3;
    }
    if((currentFlashBank == BNK512 && of.m_dwLengthUsedArea != 524288) ||
       (currentFlashBank == BNK256 && of.m_dwLengthUsedArea != 262144))
        return 3; // requested layout won't fit device - sanity check fail

    // committed to reflash now
    while(fMore) {
        VIDEO_ATTR=0xffef37;
        printk("\n\n\n\n\n\n\n\n\n           \2Updating BIOS bank...\n\2\n");
        VIDEO_ATTR=0xffffff;
        printk("           WARNING!\n"
               "           Do not turn off your console during this process!\n\n\n"
               "           Flash chip: %s\n"
               "           ManID=0x%02x , DevID=0x%02x", of.m_szFlashDescription, of.m_bManufacturerId, of.m_bDeviceId);
        if(BootFlashEraseMinimalRegion(&of)) {
            if(BootFlashProgram(&of, pbNewData)) {
                fMore=false;  // good situation

                // Set LED to oxox.
                //inputLED();

            }
            else { // failed program
                return -3;
            }
        }
        else { // failed erase
            return -2;
        }
    }
    return 0;
}

int BootFlashSettings(u8 *pbNewData, u32 dwStartOffset, u32 dwLength)
{
    OBJECT_FLASH of;
    bool fMore=true;
    bool carryOn = false;

    // A bit hacky, but easier to maintain.
    const KNOWN_FLASH_TYPE aknownflashtypesDefault[] = {
        #include "flashtypes.h"
    };

    // prep our flash object with start address and params
    of.m_pbMemoryMappedStartAddress=(u8 *)LPCFlashadress;
    of.m_dwStartOffset=dwStartOffset;
    of.m_dwLengthUsedArea=dwLength;
    of.m_pcallbackFlash=NULL;

    // check device type and parameters are sane
    if(!BootFlashGetDescriptor(&of, (KNOWN_FLASH_TYPE *)&aknownflashtypesDefault[0]))
        return 1; // unable to ID device - fail
    if(!of.m_fIsBelievedCapableOfWriteAndErase)
        return 2; // seems to be write-protected - fail
    if(of.m_dwLengthInBytes<(dwStartOffset+dwLength))
        return 3; // requested layout won't fit device - sanity check fail

    // committed to reflash now
    while(fMore) {
        if(dwStartOffset <= 0x30000)                            //Need to erase 64KB
                carryOn = BootFlashEraseMinimalRegion(&of);
        else                                                    //Only 4KB then.
                carryOn = BootFlashErase4KSector(&of);
        if(carryOn){
            if(BootFlashProgram(&of, pbNewData)) {
                fMore=false;  // good situation

                // Set LED to oxox.
                //inputLED();

            }
            else { // failed program
                return -3;
            }
        }
        else { // failed erase
            return -2;
        }
    }
    return 0;
}

void BootShowFlashDevice(void){
    //u8 class, subclass;
    //u16 vendorid, deviceid;
    //int i, j;
    // A bit hacky, but easier to maintain.
    OBJECT_FLASH of;

    const KNOWN_FLASH_TYPE aknownflashtypesDefault[] = {
        #include "flashtypes.h"
    };

    of.m_pbMemoryMappedStartAddress=(u8 *)LPCFlashadress;
/*
    if(fHasHardware == SYSCON_ID_XX1 ||
       fHasHardware == SYSCON_ID_XX2 ||
       fHasHardware == SYSCON_ID_XXOPX ||
       fHasHardware == SYSCON_ID_XX3){
        IoOutputByte(SMARTXX_FLASH_WRITEPROTECT , 1);       //Enable flash write on SmartXX mods.
    }
*/
    if(!BootFlashGetDescriptor(&of, (KNOWN_FLASH_TYPE *)&aknownflashtypesDefault[0])){
        VIDEO_ATTR=0xffc8c8c8;
        printk("No valid Flash device Detected!!!");
        return;
    }
/*
    if(fHasHardware == SYSCON_ID_XX1 ||
       fHasHardware == SYSCON_ID_XX2 ||
       fHasHardware == SYSCON_ID_XXOPX ||
       fHasHardware == SYSCON_ID_XX3){
        IoOutputByte(SMARTXX_FLASH_WRITEPROTECT , 0);       //Disable flash write on SmartXX mods.
    }
*/

    VIDEO_ATTR=0xffc8c8c8;
    printk("Manufacturer ID : ");
    VIDEO_ATTR=0xffc8c800;
    printk("%02X\n", of.m_bManufacturerId);
    VIDEO_ATTR=0xffc8c8c8;
    printk("           Device ID : ");
    VIDEO_ATTR=0xffc8c800;
    printk("%02X\n", of.m_bDeviceId);
    VIDEO_ATTR=0xffc8c8c8;
    printk("           Name : ");
    VIDEO_ATTR=0xffc8c800;
    printk("%s\n", of.m_szFlashDescription);
    VIDEO_ATTR=0xffc8c8c8;
    printk("           Total size : ");
    VIDEO_ATTR=0xffc8c800;
    printk("%u KB\n", of.m_dwLengthInBytes / 1024);

    return;
}

bool BootFlashPrintResult(int res, u32 fileSize) {
    if (res > 0) {
            cromwellError ();
            printk ("\n\n\n\n\n           Flash failed...");
            switch (res) {
                case 1:
                    printk ("\n           ");
                    cromwellError ();
                    printk ("\n           Unknown flash device.\n           Write-Protect is enabled?");
                    break;
                case 2:
                    printk ("\n           ");
                    cromwellError ();
                    printk ("\n           Cannot write to device.");
                    break;
                case 3:
                    printk ("\n           ");
                    cromwellError ();
                    printk ("\n           File size error : %u", fileSize);
                    break;
                case 4:
                    printk ("\n           ");
                    cromwellError ();
                    printk ("\n           Invalid XBlast OS update file.");
                    break;
                case 5:
                    printk ("\n           ");
                    cromwellError ();
                    printk ("\n           CRC mismatch.");
                    break;
                default:
                    printk ("\n           ");
                    cromwellError ();
                    printk ("\n           Unknown error! Congrats, you're not supposed to be here.");
                    break;
            }
        }
        else if (res == -1) {
            printk ("\n\n\n\n\n\n\n\n\n\n\n           ");
            cromwellWarning ();
            printk ("\n           Flashing aborted.");
        }
        else if (res == -2) {
            printk ("\n\n\n\n\n\n\n\n\n\n\n           ");
            cromwellWarning ();
            printk ("\n           Erasing failed, please reflash.");
        }
        else if (res == -3) {
            printk ("\n\n\n\n\n\n\n\n\n\n\n           ");
            cromwellWarning ();
            printk ("\n           Programming failed, please reflash.");
        }
        else {
            printk ("\n           ");
            cromwellSuccess ();
            printk ("\n           Flashing successful!!!");
        }
        FlashFooter ();
        return true; //Flashing is over.
}

void mirrorImage(u8 *pbNewData, u32 dwLength, OBJECT_FLASH* of){
    if(dwLength < of->m_dwLengthInBytes                          //If image size is smaller than detected flash size.
       && dwLength < 1048576){                                  //and image size is smaller than 1MB.
        if(of->m_dwLengthInBytes >= 524288 &&        //Flash is at least 512KB in space
           dwLength == 262144){                     //image is 256KB in size
            memcpy(&pbNewData[262144], &pbNewData[0], 262144);  //Mirror image in the next 256KB segment of the 1MB buffer.
            dwLength = dwLength * 2;      //Image to flash is now 512KB
        }
        if(of->m_dwLengthInBytes >= 1048576 && //Flash is at least 1MB in space
           dwLength == 524288){             //image is 512KB in size
            memcpy(&pbNewData[524288], &pbNewData[0], 524288);  //Mirror image in the next 512KB segment of the 1MB buffer.
            dwLength = dwLength * 2;      //Image to flash is now 1MB
        }
        of->m_dwLengthUsedArea=dwLength;
    }
}
