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

// A bit hacky, but easier to maintain.
const KNOWN_FLASH_TYPE aknownflashtypesDefault[] = {
#include "flashtypes.h"
};

 // callback to show progress
bool BootFlashUserInterface(void * pvoidObjectFlash, ENUM_EVENTS ee, u32 dwPos, u32 dwExtent) {
	if(ee==EE_ERASE_UPDATE){
		DisplayProgressBar(dwPos,dwExtent,0xffffff00);
	}
	else if(ee==EE_PROGRAM_UPDATE){
		DisplayProgressBar(dwPos,dwExtent,0xff00ff00);
	}
	return true;
}

int BootReflashAndReset(u8 *pbNewData, u32 dwStartOffset, u32 dwLength)
{
	OBJECT_FLASH of;
	bool fMore=true;

	// prep our flash object with start address and params
	of.m_pbMemoryMappedStartAddress=(u8 *)LPCFlashadress;
	of.m_dwStartOffset=dwStartOffset;
	of.m_dwLengthUsedArea=dwLength;
	of.m_pcallbackFlash=BootFlashUserInterface;

	// check device type and parameters are sane
	if(!BootFlashGetDescriptor(&of, (KNOWN_FLASH_TYPE *)&aknownflashtypesDefault[0])) return 1; // unable to ID device - fail
	if(!of.m_fIsBelievedCapableOfWriteAndErase) return 2; // seems to be write-protected - fail
	if(of.m_dwLengthInBytes<(dwStartOffset+dwLength)) return 3; // requested layout won't fit device - sanity check fail
	
	// committed to reflash now
	while(fMore) {
		VIDEO_ATTR=0xffef37;
		printk("\n\n\n\n\n\n\n\n\n\n\n\n\n           \2Flashing BIOS...\n\2\n");
		VIDEO_ATTR=0xffffff;
		printk("           WARNING!\n"
				 "           Do not turn off your console during this process!\n"
				 "           Your console should automatically reboot when this\n"
				 "           is done.  However, if it does not, please manually\n"
				 "           do so by pressing the power button once the LED has\n"
				 "           turned flashing amber (oxox)\n");

		if(BootFlashEraseMinimalRegion(&of)) {
			if(BootFlashProgram(&of, pbNewData)) {
				fMore=false;  // good situation

				// Set LED to oxox.
				inputLED();

				I2CRebootSlow();
				while(1);

			} else { // failed program
				printk("Programming failed...\n");
				while(1);
			}
		} else { // failed erase
			printk("Erasing failed...\n");
			while(1);
		}
	}
	return 0; // keep compiler happy
}
