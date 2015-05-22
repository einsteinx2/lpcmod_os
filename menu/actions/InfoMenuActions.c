/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "boot.h"
#include "video.h"
#include "xbox.h"
#include "BootEEPROM.h"
#include "BootFlash.h"
#include "InfoMenuActions.h"
#include "lib/LPCMod/BootLPCMod.h"


void ShowTemperature(void *whatever) {
    int c, cx;
    int f, fx;
    InfoHeader("Temperature");
    I2CGetTemperature(&c, &cx);
    f = ((9.0 / 5.0) * c) + 32.0;
    fx = ((9.0 / 5.0) * cx) + 32.0;
    VIDEO_ATTR=0xffc8c8c8;
    printk("CPU temperature: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%d°C / %d°F\n           ", c, f);
    VIDEO_ATTR=0xffc8c8c8;
    printk("Board temperature: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%d°C / %d°F\n", cx, fx);
    UIFooter();
}

void ShowVideo(void *whatever) {
    InfoHeader("Video");
    VIDEO_ATTR=0xffc8c8c8;
    printk("Encoder: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%s  ", VideoEncoderName());
    VIDEO_ATTR=0xffc8c8c8;
    printk("Cable: ");
    VIDEO_ATTR=0xffc8c800;
    printk("%s  ", AvCableName());
    UIFooter();
}

void ShowEeprom(void *whatever) {
    InfoHeader("EEPROM");
    BootEepromPrintInfo();
    UIFooter();
}

void ShowFlashChip(void *whatever) {
    InfoHeader("Flash device");
    BootShowFlashDevice();
    UIFooter();
}

void InfoHeader(char *title) {
    printk("\n\n\n\n\n");
    VIDEO_ATTR=0xffffef37;
    printk("\2%s information:\2\n\n\n\n\n\n\n\n           ", title);
    VIDEO_ATTR=0xffc8c8c8;
}

void ShowUncommittedChanges(void *whatever){
#define MAXITEMSINONESCREEN 20
    char printString[200];
    u8 UncommittedChanges = LPCMod_CountNumberOfChangesInSettings();
    u8 NbOfItemsToList = UncommittedChanges;
    bool tooMuchItems = false;
    u8 i, j, k;
    u8 numberOfEEPROMChanges;
    bool IPChange;
    _settingsPtrStruct originalSettingsPtrStruct;
    setCFGFileTransferPtr(&LPCmodSettingsOrigFromFlash, &originalSettingsPtrStruct);
    printk("\n\n");
    VIDEO_ATTR=0xffffef37;
    printk("\2Uncommitted change%s information:\2\n\n", UncommittedChanges > 1 ? "s" : "");
    VIDEO_ATTR=0xffc8c8c8;

    if(UncommittedChanges == 0){
    	printk("\n           No uncommitted change.");
    }
    else{
    	if(UncommittedChanges > MAXITEMSINONESCREEN){
    		NbOfItemsToList = MAXITEMSINONESCREEN;
    		tooMuchItems = true;
    	}

    	for(i = 0; i < MAXEDITABLEPARAMSINEEPROM; i++){
    	    eepromChangesStringArray[i] = NULL;
    	}

    	numberOfEEPROMChanges = generateStringsForEEPROMChanges(true);

    	if(numberOfEEPROMChanges > 0){
    	    for(i = 0; i < numberOfEEPROMChanges && i < MAXITEMSINONESCREEN; i++){
    	        if(eepromChangesStringArray[i] != NULL){
                    printk("\n       %s", eepromChangesStringArray[i]);
                    free(eepromChangesStringArray[i]);
    	        }
    	    }
            NbOfItemsToList -= numberOfEEPROMChanges > MAXITEMSINONESCREEN ? MAXITEMSINONESCREEN : numberOfEEPROMChanges;
    	}

        if(LPCMod_checkForBootScriptChanges() && NbOfItemsToList > 0){
            printk("\n       Boot script in flash modified");
            NbOfItemsToList -= 1;
        }

        k = 0;
		for(i = 0; (i < NBTXTPARAMS) && (k < NbOfItemsToList); i++){
			if(i < IPTEXTPARAMGROUP){
				if(*originalSettingsPtrStruct.settingsPtrArray[i] != *settingsPtrStruct.settingsPtrArray[i]){
					if(i < NBBOOLEANPARAMS){
						printk("\n       %s \"%s\" -> \"%s\"", xblastcfgstrings[i], *originalSettingsPtrStruct.settingsPtrArray[i] ? "Yes" : "No", *settingsPtrStruct.settingsPtrArray[i] ? "Yes" : "No");
					}
					else{
						printk("\n       %s \"%u\" -> \"%u\"", xblastcfgstrings[i], *originalSettingsPtrStruct.settingsPtrArray[i], *settingsPtrStruct.settingsPtrArray[i]);
					}

					k += 1;
				}
			}
			else if(i < TEXTPARAMGROUP){
				for(j = 0; j < 4; j++){
					if(originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][j] != settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][j]){

						printk("\n       %s \"%u.%u.%u.%u\" -> \"%u.%u.%u.%u\"", xblastcfgstrings[i],
										originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][0],
										originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][1],
										originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][2],
										originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][3],
										settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][0],
										settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][1],
										settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][2],
										settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][3]);

						k += 1;
						break;
					}
				}
			}
			else if(i < SPECIALPARAMGROUP){
				if(strcmp(originalSettingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP], settingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP])){
					printk("\n       %s \"%s\" -> \"%s\"", xblastcfgstrings[i], originalSettingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP], settingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP]);

					k += 1;
				}
			}
			else{
				if(*originalSettingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP] != *settingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP]){
					printk("\n       %s \"%u\" -> \"%u\"", xblastcfgstrings[i], *originalSettingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP], *settingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP]);

					k += 1;
				}
			}
		}

    	if(tooMuchItems){
            printk("\n       Too much uncommitted changes to list them all...");
    	}
    }
    UIFooter();
}
