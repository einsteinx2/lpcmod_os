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
#include "stdio.h"
#include "stdlib.h"
#include <stddef.h>
#include <limits.h>

static const char* const ChangeStringLabelSeparator = " : ";
static const char* const ChangeStringArrow          = " -> ";
#define SimpleChangeStringSizeCalc(x) ((2 * x) + strlen(ChangeStringArrow) + strlen(ChangeStringLabelSeparator) + sizeof("\0") + (4 * sizeof('\"')))

#define ByteStringSize 3
#define NbBytesPerIP    4
#define IPChangeStringSizeCalc() ((2 * (NbBytesPerIP * ByteStringSize + 3 * sizeof('.'))) + strlen(ChangeStringArrow) + strlen(ChangeStringLabelSeparator) + sizeof("\0") + (4 * sizeof('\"')))

#define TextChangeStringSizeCalc(old, new) (strlen(old) + strlen(new) + strlen(ChangeStringArrow) + strlen(ChangeStringLabelSeparator) + sizeof("\0") + (4 * sizeof('\"')))

#define SpecialChangeStringSizeCalc(index) (strlen(getSpecialSettingString(index, *originalSettingsPtrStruct.specialCasePtrArray[index])) + strlen(getSpecialSettingString(index, *settingsPtrStruct.specialCasePtrArray[index])) + strlen(ChangeStringArrow) + strlen(ChangeStringLabelSeparator) + sizeof("\0") + (4 * sizeof('\"')))

#define SimpleChangeStringGen(setting, type) sprintf(currentChangeEntry->changeString, "%s\""type"\"%s\""type"\"", \
                                                        ChangeStringLabelSeparator, \
                                                        *originalSettingsPtrStruct.setting, \
                                                        ChangeStringArrow, *settingsPtrStruct.setting);

#define IPChangeStringGen(index)  sprintf(currentChangeEntry->changeString, "%s\"%u.%u.%u.%u\"%s\"%u.%u.%u.%u\"", \
                                        ChangeStringLabelSeparator, \
                                        originalSettingsPtrStruct.IPsettingsPtrArray[index][0], \
                                        originalSettingsPtrStruct.IPsettingsPtrArray[index][1], \
                                        originalSettingsPtrStruct.IPsettingsPtrArray[index][2], \
                                        originalSettingsPtrStruct.IPsettingsPtrArray[index][3], \
                                        ChangeStringArrow, \
                                        settingsPtrStruct.IPsettingsPtrArray[index][0], \
                                        settingsPtrStruct.IPsettingsPtrArray[index][1], \
                                        settingsPtrStruct.IPsettingsPtrArray[index][2], \
                                        settingsPtrStruct.IPsettingsPtrArray[index][3]);

#define TextChangeStringGen(setting, type) sprintf(currentChangeEntry->changeString, "%s\""type"\"%s\""type"\"", \
                                                        ChangeStringLabelSeparator, \
                                                        originalSettingsPtrStruct.setting, \
                                                        ChangeStringArrow, settingsPtrStruct.setting);

#define SpecialChangeStringGen(index) sprintf(currentChangeEntry->changeString, "%s\"%s\"%s\"%s\"", \
                                                        ChangeStringLabelSeparator, \
                                                        getSpecialSettingString(index, *originalSettingsPtrStruct.specialCasePtrArray[index]), \
                                                        ChangeStringArrow, \
                                                        getSpecialSettingString(index, *settingsPtrStruct.specialCasePtrArray[index]));

#define AddChange() if(currentChangeEntry != NULL) putNewChangeInList(output, currentChangeEntry); numberOfChanges += 1;


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
    char testTypeLength[30];

    unsigned char i, j;
    _settingsPtrStruct originalSettingsPtrStruct;
    setCFGFileTransferPtr(&LPCmodSettingsOrigFromFlash, &originalSettingsPtrStruct);
    setCFGFileTransferPtr(&LPCmodSettings, &settingsPtrStruct);

    for(i = 0; i < BoolParamGroup; i++)
    {
        currentChangeEntry = NULL;
        if(*originalSettingsPtrStruct.boolSettingsPtrArray[i] != *settingsPtrStruct.boolSettingsPtrArray[i])
        {
            if(generateChangeStruct)
            {
                currentChangeEntry = malloc(sizeof(OSSettingsChangeEntry_t));
                currentChangeEntry->label = xblastCfgStringsStruct.boolSettingsStringArray[i];
                sprintf(testTypeLength, "%u", UCHAR_MAX);
                currentChangeEntry->changeString = malloc(SimpleChangeStringSizeCalc(strlen(testTypeLength)));
                SimpleChangeStringGen(boolSettingsPtrArray[i], "%u");
                currentChangeEntry->origSettings = originalSettingsPtrStruct.boolSettingsPtrArray[i];
                currentChangeEntry->newSetting = settingsPtrStruct.boolSettingsPtrArray[i];
                currentChangeEntry->settingSize = 1;
            }
            AddChange();
        }
    }

    for(i = 0; i < NumParamGroup; i++)
    {
        currentChangeEntry = NULL;
        if(*originalSettingsPtrStruct.numSettingsPtrArray[i] != *settingsPtrStruct.numSettingsPtrArray[i])
        {
            if(generateChangeStruct)
            {
                currentChangeEntry = malloc(sizeof(OSSettingsChangeEntry_t));
                currentChangeEntry->label = xblastCfgStringsStruct.numSettingsStringArray[i];
                sprintf(testTypeLength, "%u", UCHAR_MAX);
                currentChangeEntry->changeString = malloc(SimpleChangeStringSizeCalc(strlen(testTypeLength)));
                SimpleChangeStringGen(numSettingsPtrArray[i], "%u");
                currentChangeEntry->origSettings = originalSettingsPtrStruct.numSettingsPtrArray[i];
                currentChangeEntry->newSetting = settingsPtrStruct.numSettingsPtrArray[i];
                currentChangeEntry->settingSize = 1;
            }
            AddChange();
        }
    }

    for(i = 0; i < IPParamGroup; i++)
    {
        if(0 != memcmp(originalSettingsPtrStruct.IPsettingsPtrArray[i], settingsPtrStruct.IPsettingsPtrArray[i], 4))
        {
            if(generateChangeStruct)
            {
                currentChangeEntry = malloc(sizeof(OSSettingsChangeEntry_t));
                currentChangeEntry->label = xblastCfgStringsStruct.IPsettingsStringArray[i];
                currentChangeEntry->changeString = malloc(IPChangeStringSizeCalc());
                IPChangeStringGen(i);
                currentChangeEntry->origSettings = originalSettingsPtrStruct.IPsettingsPtrArray[i];
                currentChangeEntry->newSetting = settingsPtrStruct.IPsettingsPtrArray[i];
                currentChangeEntry->settingSize = 4;
            }
            AddChange();
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
                currentChangeEntry = malloc(sizeof(OSSettingsChangeEntry_t));
                currentChangeEntry->label = xblastCfgStringsStruct.textSettingsStringArray[i];
                currentChangeEntry->changeString = malloc(TextChangeStringSizeCalc(originalSettingsPtrStruct.textSettingsPtrArray[i], settingsPtrStruct.textSettingsPtrArray[i]));
                TextChangeStringGen(textSettingsPtrArray[i], "%s");
                currentChangeEntry->origSettings = originalSettingsPtrStruct.textSettingsPtrArray[i];
                currentChangeEntry->newSetting = settingsPtrStruct.textSettingsPtrArray[i];
                currentChangeEntry->settingSize = origLength > newLength ? origLength : newLength;
            }
            AddChange();
        }
    }

    for(i = 0; i < SpecialParamGroup; i++)
    {
        if(*originalSettingsPtrStruct.specialCasePtrArray[i] != *settingsPtrStruct.specialCasePtrArray[i])
        {
            if(generateChangeStruct)
            {
                currentChangeEntry = malloc(sizeof(OSSettingsChangeEntry_t));
                currentChangeEntry->label = xblastCfgStringsStruct.specialSettingsStringArray[i];
                currentChangeEntry->changeString = malloc(SpecialChangeStringSizeCalc(i));
                SpecialChangeStringGen(i);
                currentChangeEntry->origSettings = originalSettingsPtrStruct.specialCasePtrArray[i];
                currentChangeEntry->newSetting = settingsPtrStruct.specialCasePtrArray[i];
                currentChangeEntry->settingSize = 1;
            }
            AddChange();
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
        free(curEntry->changeString);
        free(curEntry);
        curEntry = nextEntry;
    }

    input->changeCount = 0;
    input->firstChangeEntry = NULL;
}

static void putNewChangeInList(OSSettingsChangeList* list, OSSettingsChangeEntry_t* change)
{
    OSSettingsChangeEntry_t* cycler = list->firstChangeEntry;

    change->nextChange = NULL;

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
