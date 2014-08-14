/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "SystemMenuActions.h"
#include "boot.h"

void incrementFanSpeed(void * itemStr){
	if(LPCmodSettings.OSsettings.fanSpeed < 98)	//Logic
		LPCmodSettings.OSsettings.fanSpeed += 2;	//Actual value range of PIC is from 0 to 50 which maps to 0 to 100 here.
	sprintf(itemStr, "Fan speed : %d%%", LPCmodSettings.OSsettings.fanSpeed);
	I2CSetFanSpeed(LPCmodSettings.OSsettings.fanSpeed);	//Send new speed to PIC
}

void decrementFanSpeed(void * itemStr){
	if(LPCmodSettings.OSsettings.fanSpeed > 12)	//For now, I won't allow anyone to set their fan below 10% speed. Come on...
		LPCmodSettings.OSsettings.fanSpeed -= 2;
	sprintf(itemStr, "Fan speed : %d%%", LPCmodSettings.OSsettings.fanSpeed);
	I2CSetFanSpeed(LPCmodSettings.OSsettings.fanSpeed);	//Send new speed to PIC
}

void incrementGameRegion(void * itemStr){
	char *Gameregiontext[5] = {
			"Unknown/Error",
			"NTSC-U",
			"NTSC-J",
			"n/a",
			"PAL"
		};
	int gameRegion = getGameRegionValue();
	switch(gameRegion){
		case NORTH_AMERICA:
			setGameRegionValue(JAPAN);
			break;
		case JAPAN:
			setGameRegionValue(EURO_AUSTRALIA);
			break;
		case EURO_AUSTRALIA:
			setGameRegionValue(NORTH_AMERICA);
			break;
		default:
			break;
		}
	sprintf(itemStr, "Game region : %s",Gameregiontext[gameRegion]);
}

void decrementGameRegion(void * itemStr){
	char *Gameregiontext[5] = {
			"Unknown/Error",
			"NTSC-U",
			"NTSC-J",
			"n/a",
			"PAL"
		};
	int gameRegion = getGameRegionValue();
	switch(gameRegion){
		case NORTH_AMERICA:
			setGameRegionValue(EURO_AUSTRALIA);
			break;
		case JAPAN:
			setGameRegionValue(NORTH_AMERICA);
			break;
		case EURO_AUSTRALIA:
			setGameRegionValue(JAPAN);
			break;
		}
	sprintf(itemStr, "Game region : %s",Gameregiontext[gameRegion]);
}

void incrementDVDRegion(void * itemStr){
	//String enum for DVD_ZONE
	char *DVDregiontext[9] = {
		"Region Clear",
		"USA (1)",
		"Europe (2)",
		"India (3)",
		"Australia (4)",
		"USSR (5)",
		"China (6)",
		"Free (7)",
		"Airlines (8)"
	};
	if(eeprom.DVDPlaybackKitZone[0] < 8)
		eeprom.DVDPlaybackKitZone[0] += 1;
	sprintf(itemStr, "DVD region : %s",DVDregiontext[eeprom.DVDPlaybackKitZone[0]]);
	EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}

void decrementDVDRegion(void * itemStr){
	//String enum for DVD_ZONE
	char *DVDregiontext[9] = {
		"Region Clear",
		"USA (1)",
		"Europe (2)",
		"India (3)",
		"Australia (4)",
		"USSR (5)",
		"China (6)",
		"Free (7)",
		"Airlines (8)"
	};
	if(eeprom.DVDPlaybackKitZone[0]  > 0)
		eeprom.DVDPlaybackKitZone[0] -= 1;
	sprintf(itemStr, "DVD region : %s",DVDregiontext[eeprom.DVDPlaybackKitZone[0]]);
	EepromCRC(eeprom.Checksum3,eeprom.TimeZoneBias,0x5b);
}
