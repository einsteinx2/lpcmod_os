/*
 * xblastSettingsChangeTracker.c
 *
 *  Created on: Aug 11, 2016
 *      Author: cromwelldev
 */

#ifndef XBLASTSETTINGSCHANGETRACKER_C_
#define XBLASTSETTINGSCHANGETRACKER_C_

#include "xblastSettingsChangeTracker.h"
#include "xblastSettingsImportExport.h"
#include "xblastSettings.h"
#include "xblast/scriptEngine/xblastScriptEngine.h"
#include "lib/LPCMod/xblastDebug.h"
#include "string.h"
#include "stdlib.h"
#include <stddef.h>

static void putNewChangeInList(OSSettingsChangeList* list, OSSettingsChangeEntry_t* change);

void settingsTrackerInit(void)
{
    osSettingsChangeList.changeCount = 0;
    osSettingsChangeList.firstChangeEntry = NULL;
}

unsigned char LPCMod_CountNumberOfChangesInSettings(bool generateChangeStruct, OSSettingsChangeList* output)
{
    int numberOfChanges = 0;
    OSSettingsChangeEntry_t* currentChangeEntry;
    output->changeCount = 0;

    unsigned char i, j;
    _settingsPtrStruct originalSettingsPtrStruct;
    setCFGFileTransferPtr(&LPCmodSettingsOrigFromFlash, &originalSettingsPtrStruct);

    for(i = 0; i < NBTXTPARAMS; i++)
    {
        currentChangeEntry = NULL;
        if(i < IPTEXTPARAMGROUP)
        {
            if(*originalSettingsPtrStruct.settingsPtrArray[i] != *settingsPtrStruct.settingsPtrArray[i])
            {
                if(generateChangeStruct)
                {
                    currentChangeEntry = calloc(1, sizeof(OSSettingsChangeEntry_t));
                    currentChangeEntry->label = xblastcfgstrings[i];
                    sprintf(currentChangeEntry->changeString, "%u -> %u", *originalSettingsPtrStruct.settingsPtrArray[i], *settingsPtrStruct.settingsPtrArray[i]);
                    currentChangeEntry->origSettings = originalSettingsPtrStruct.settingsPtrArray[i];
                    currentChangeEntry->newSetting = settingsPtrStruct.settingsPtrArray[i];
                    currentChangeEntry->settingSize = 1;
                }
                numberOfChanges += 1;
            }
        }
        else if(i < TEXTPARAMGROUP)
        {
            for(j = 0; j < 4; j++)
            {
                if(originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][j] != settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][j])
                {
                    if(generateChangeStruct)
                    {
                        currentChangeEntry = calloc(1, sizeof(OSSettingsChangeEntry_t));
                        currentChangeEntry->label = xblastcfgstrings[i];
                        sprintf(currentChangeEntry->changeString, "\"%u.%u.%u.%u\" -> \"%u.%u.%u.%u\"",
                                    originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][0],
                                    originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][1],
                                    originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][2],
                                    originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][3],
                                    settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][0],
                                    settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][1],
                                    settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][2],
                                    settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][3]);
                        currentChangeEntry->origSettings = originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP];
                        currentChangeEntry->newSetting = settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP];
                        currentChangeEntry->settingSize = 4;
                    }
                    numberOfChanges += 1;
                    break;
                }
            }
        }
        else if(i < SPECIALPARAMGROUP)
        {
            unsigned char origLength = strlen(originalSettingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP]);
            unsigned char newLength = strlen(settingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP]);
            if(origLength != newLength ||
               strcmp(originalSettingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP], settingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP]))
            {
                if(generateChangeStruct)
                {
                    currentChangeEntry = calloc(1, sizeof(OSSettingsChangeEntry_t));
                    currentChangeEntry->label = xblastcfgstrings[i];
                    sprintf(currentChangeEntry->changeString, "\"%s\" -> \"%s\"", originalSettingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP], settingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP]);
                    currentChangeEntry->origSettings = originalSettingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP];
                    currentChangeEntry->newSetting = settingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP];
                    currentChangeEntry->settingSize = origLength > newLength ? origLength : newLength;
                }
                numberOfChanges += 1;
            }
        }
        else
        {
            if(*originalSettingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP] != *settingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP])
            {
                if(generateChangeStruct)
                {
                    currentChangeEntry = calloc(1, sizeof(OSSettingsChangeEntry_t));
                    currentChangeEntry->label = xblastcfgstrings[i];
                    const char* origString = getSpecialSettingString(i - SPECIALPARAMGROUP, *originalSettingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP]);
                    const char* newString = getSpecialSettingString(i - SPECIALPARAMGROUP, *settingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP]);
                    sprintf(currentChangeEntry->changeString, "\"%s\" -> \"%s\"", origString, newString);
                    currentChangeEntry->origSettings = originalSettingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP];
                    currentChangeEntry->newSetting = settingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP];
                    currentChangeEntry->settingSize = 1;
                }
                numberOfChanges += 1;
            }
        }

        if(currentChangeEntry != NULL)
        {
            putNewChangeInList(output, currentChangeEntry);
        }
    }

    return numberOfChanges;
}


bool LPCMod_checkForBootScriptChanges(void){
    unsigned int oldBufferSize, modifiedBufferSize;

    //If there's both a new script to save and a script already saved in flash. Check if they are different.
    modifiedBufferSize = LPCmodSettings.flashScript.scriptSize;
    oldBufferSize = LPCmodSettingsOrigFromFlash.flashScript.scriptSize;

    if(modifiedBufferSize != oldBufferSize){    //Scripts size differ.
        return true;
    }

    //If above condition is false, it means both script have the same size
    if(memcmp(LPCmodSettings.flashScript.scriptData, LPCmodSettingsOrigFromFlash.flashScript.scriptData, ScriptSavedInFlashMaxSizeInBytes)){
        return true;
    }

    //Both scripts are identical.
    return false;
}

bool LPCMod_checkForBackupEEPROMChange(void)
{
    if(memcmp(&LPCmodSettings.bakeeprom, &LPCmodSettingsOrigFromFlash.bakeeprom, sizeof(EEPROMDATA)))
    {
        return true;
    }

    //Both EEPROM backups are identical.
    return false;
}

void cleanOSSettingsChangeListStruct(OSSettingsChangeList* input)
{
    OSSettingsChangeEntry_t* nextEntry;
    OSSettingsChangeEntry_t* curEntry = input->firstChangeEntry;

    while(curEntry != NULL)
    {
        nextEntry = curEntry->nextChange;
        free(curEntry);
        curEntry = nextEntry;
    }

    input->changeCount = 0;
    input->firstChangeEntry = NULL;
}

static void putNewChangeInList(OSSettingsChangeList* list, OSSettingsChangeEntry_t* change)
{
    OSSettingsChangeEntry_t* cycler = list->firstChangeEntry;

    if(list->firstChangeEntry == NULL)
    {
        list->firstChangeEntry = change;
        list->changeCount = 1;
        return;
    }

    while(cycler->nextChange != NULL)
    {
        cycler = cycler->nextChange;
    }

    list->changeCount += 1;
    cycler->nextChange = change;
}

#endif /* XBLASTSETTINGSCHANGETRACKER_C_ */
