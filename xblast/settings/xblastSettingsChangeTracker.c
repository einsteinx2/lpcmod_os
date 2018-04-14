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

    for(i = 0; i < BoolParamGroup; i++)
    {
        currentChangeEntry = NULL;
        if(*originalSettingsPtrStruct.boolSettingsPtrArray[i] != *settingsPtrStruct.boolSettingsPtrArray[i])
        {
            if(generateChangeStruct)
            {
                currentChangeEntry = calloc(1, sizeof(OSSettingsChangeEntry_t));
                currentChangeEntry->label = xblastCfgStringsStruct.boolSettingsStringArray[i];
                sprintf(currentChangeEntry->changeString, "%u -> %u", *originalSettingsPtrStruct.boolSettingsPtrArray[i], *settingsPtrStruct.boolSettingsPtrArray[i]);
                currentChangeEntry->origSettings = originalSettingsPtrStruct.boolSettingsPtrArray[i];
                currentChangeEntry->newSetting = settingsPtrStruct.boolSettingsPtrArray[i];
                currentChangeEntry->settingSize = 1;
            }
            numberOfChanges += 1;
        }
    }

    for(i = 0; i < NumParamGroup; i++)
    {
        currentChangeEntry = NULL;
        if(*originalSettingsPtrStruct.numSettingsPtrArray[i] != *settingsPtrStruct.numSettingsPtrArray[i])
        {
            if(generateChangeStruct)
            {
                currentChangeEntry = calloc(1, sizeof(OSSettingsChangeEntry_t));
                currentChangeEntry->label = xblastCfgStringsStruct.numSettingsStringArray[i];
                sprintf(currentChangeEntry->changeString, "%u -> %u", *originalSettingsPtrStruct.numSettingsPtrArray[i], *settingsPtrStruct.numSettingsPtrArray[i]);
                currentChangeEntry->origSettings = originalSettingsPtrStruct.numSettingsPtrArray[i];
                currentChangeEntry->newSetting = settingsPtrStruct.numSettingsPtrArray[i];
                currentChangeEntry->settingSize = 1;
            }
            numberOfChanges += 1;
        }
    }

    for(i = 0; i < IPParamGroup; i++)
    {
        if(0 != memcmp(originalSettingsPtrStruct.IPsettingsPtrArray[i], settingsPtrStruct.IPsettingsPtrArray[i], 4))
        {
            if(generateChangeStruct)
            {
                currentChangeEntry = calloc(1, sizeof(OSSettingsChangeEntry_t));
                currentChangeEntry->label = xblastCfgStringsStruct.IPsettingsStringArray[i];
                sprintf(currentChangeEntry->changeString, "\"%u.%u.%u.%u\" -> \"%u.%u.%u.%u\"",
                            originalSettingsPtrStruct.IPsettingsPtrArray[i][0],
                            originalSettingsPtrStruct.IPsettingsPtrArray[i][1],
                            originalSettingsPtrStruct.IPsettingsPtrArray[i][2],
                            originalSettingsPtrStruct.IPsettingsPtrArray[i][3],
                            settingsPtrStruct.IPsettingsPtrArray[i][0],
                            settingsPtrStruct.IPsettingsPtrArray[i][1],
                            settingsPtrStruct.IPsettingsPtrArray[i][2],
                            settingsPtrStruct.IPsettingsPtrArray[i][3]);
                currentChangeEntry->origSettings = originalSettingsPtrStruct.IPsettingsPtrArray[i];
                currentChangeEntry->newSetting = settingsPtrStruct.IPsettingsPtrArray[i];
                currentChangeEntry->settingSize = 4;
            }
            numberOfChanges += 1;
            break;
        }
    }



    for(i = 0; i < TextParamGroup; i++)
    {
        unsigned char origLength = strlen(originalSettingsPtrStruct.textSettingsPtrArray[i]);
        unsigned char newLength = strlen(settingsPtrStruct.textSettingsPtrArray[i]);
        if(origLength != newLength ||
           strcmp(originalSettingsPtrStruct.textSettingsPtrArray[i], settingsPtrStruct.textSettingsPtrArray[i]))
        {
            if(generateChangeStruct)
            {
                currentChangeEntry = calloc(1, sizeof(OSSettingsChangeEntry_t));
                currentChangeEntry->label = xblastCfgStringsStruct.textSettingsStringArray[i];
                sprintf(currentChangeEntry->changeString, "\"%s\" -> \"%s\"", originalSettingsPtrStruct.textSettingsPtrArray[i], settingsPtrStruct.textSettingsPtrArray[i]);
                currentChangeEntry->origSettings = originalSettingsPtrStruct.textSettingsPtrArray[i];
                currentChangeEntry->newSetting = settingsPtrStruct.textSettingsPtrArray[i];
                currentChangeEntry->settingSize = origLength > newLength ? origLength : newLength;
            }
            numberOfChanges += 1;
        }
    }

    for(i = 0; i < SpecialParamGroup; i++)
    {
        if(*originalSettingsPtrStruct.specialCasePtrArray[i] != *settingsPtrStruct.specialCasePtrArray[i])
        {
            if(generateChangeStruct)
            {
                currentChangeEntry = calloc(1, sizeof(OSSettingsChangeEntry_t));
                currentChangeEntry->label = xblastCfgStringsStruct.specialSettingsStringArray[i];
                const char* const origString = getSpecialSettingString(i, *originalSettingsPtrStruct.specialCasePtrArray[i]);
                const char* const newString = getSpecialSettingString(i, *settingsPtrStruct.specialCasePtrArray[i]);
                sprintf(currentChangeEntry->changeString, "\"%s\" -> \"%s\"", origString, newString);
                currentChangeEntry->origSettings = originalSettingsPtrStruct.specialCasePtrArray[i];
                currentChangeEntry->newSetting = settingsPtrStruct.specialCasePtrArray[i];
                currentChangeEntry->settingSize = 1;
            }
            numberOfChanges += 1;
        }
    }

    if(currentChangeEntry != NULL)
    {
        putNewChangeInList(output, currentChangeEntry);
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
