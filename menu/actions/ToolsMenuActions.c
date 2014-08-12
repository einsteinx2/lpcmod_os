/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ToolsMenuActions.h"
#include "lpcmod_v1.h"
#include "boot.h"


void saveEEPromToFlash(void *whatever){
	memcpy(&(LPCmodSettings.bakeeprom),&eeprom,sizeof(EEPROMDATA));
}

void restoreEEPromFromFlash(void *whatever){
	u8 i;
	u8 emptyCount = 0;
	for(i = 0; i < 4; i++) {	//Checksum2 is 4 bytes long.
		if(LPCmodSettings.bakeeprom.Checksum2[i] == 0xFF)
			emptyCount++;
	}
	if(emptyCount < 4)			//Make sure checksum2 is not 0xFFFFFFFF. It is practically impossible to get such value as a checsum.
		memcpy(&eeprom,&(LPCmodSettings.bakeeprom),sizeof(EEPROMDATA));
}

void wipeEEPromUserSettings(void *whatever){
	memset(eeprom.Checksum3,0xFF,4);	//Checksum3 need to be 0xFFFFFFFF
	memset(eeprom.TimeZoneBias,0x00,0x5b);	//Start from Checksum3 address in struct and write 0x00 up to UNKNOWN6.
}
