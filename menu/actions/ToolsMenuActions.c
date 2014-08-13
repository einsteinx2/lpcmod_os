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

void memtest(void * whatever){
	u8 bank = 0;
	char Bank1Text[50];
	char Bank2Text[50];
	char Bank3Text[50];
	char Bank4Text[50];
	char *BankText[4] = {Bank1Text, Bank2Text, Bank3Text, Bank4Text};
	strcpy(Bank1Text,"Untested");
	strcpy(Bank2Text,"Untested");
	strcpy(Bank3Text,"Untested");
	strcpy(Bank4Text,"Untested");
	PciWriteDword(BUS_0, DEV_0, FUNC_0, 0x84, 0x7FFFFFF);  //Force 128 MB
	for(bank = 0; bank < 4; bank++)	{
		strcpy(BankText[bank],testBank(bank)? "Failed" : "Success");
		DisplayProgressBar(bank,4,0xff00ff00);
		wait_ms(1000);
	}
	if (xbox_ram == 64) {	//Revert to 64MB RAM
		PciWriteDword(BUS_0, DEV_0, FUNC_0, 0x84, 0x3FFFFFF);  // 64 MB
	}
	ToolFooter();
}

void ToolFooter(void) {
	VIDEO_ATTR=0xffc8c8c8;
	printk("\n\n           Press Button 'A' to continue.");
	while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
}

int testBank(int bank){
	u32 counter, lastValue;
	u32 *membasetop = (u32*)((64*1024*1024));
	u8 result=0;

	lastValue = 1;
	//Clear Upper 64MB
	for (counter= 0; counter< (64*1024*1024/(4*4));counter++) {
		membasetop[counter*4+bank] = lastValue;
	}

	while(lastValue < 0x80000000){
		for (counter= 0; counter< (64*1024*1024/(4*4));counter++) {
			if(membasetop[counter*4+bank]!=lastValue){
				result = 1;
				lastValue = 0x80000000;
				break;		//No need to go further. Bank is broken.
			}
			membasetop[counter*4+bank] = lastValue<<1;
		}
		lastValue = lastValue << 1;
	}
	return result;
}
