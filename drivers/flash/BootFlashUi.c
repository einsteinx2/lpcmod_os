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
    of.m_dwStartOffset=dwStartOffset;
    of.m_dwLengthUsedArea=dwLength;
    of.m_pcallbackFlash=BootFlashUserInterface;

    // check device type and parameters are sane
    if(!BootFlashGetDescriptor(&of, (KNOWN_FLASH_TYPE *)&aknownflashtypesDefault[0]))
        return 1; // unable to ID device - fail
    if(!of.m_fIsBelievedCapableOfWriteAndErase)
        return 2; // seems to be write-protected - fail
    if(of.m_dwLengthInBytes<(dwStartOffset+dwLength))
        return 3; // requested layout won't fit device - sanity check fail
    if(fHasHardware == SYSCON_ID_V1){			//Only check when on a XBlast mod. For the rest, I don't care.
        if(assertOSUpdateValidInput(pbNewData))
            return 4;  //Not valid XBlast OS image.
        printk("\n              CRC32 = 0x%08X", crc32buf(pbNewData,0x3F000));
        printk("\n             in BIN = 0x%08X", (u32 *)&pbNewData[0x3FDFC]);
        while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
        //FIXME: Enable once proven working.
        /*
        if(crc32buf(pbNewData,0x3F000) != (u32 *)&pbNewData[0x3FDFC])
            return 5;
         */
    }
    
    // committed to reflash now
    while(fMore) {
        VIDEO_ATTR=0xffef37;
        printk("\n\n\n\n\n\n\n\n\n           \2Updating XBlast OS...\n\2\n");
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
                return -3;
            }
        }
        else { // failed erase
            //printk("           Erasing failed...\n");
            return -2;
        }
    }
    return 0; // keep compiler happy
}

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
    if(of.m_dwLengthInBytes<(dwStartOffset+dwLength))
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
