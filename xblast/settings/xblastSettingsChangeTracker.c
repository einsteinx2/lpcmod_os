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
#include "xblastSettingsDefs.h"
#include "xblast/scriptEngine/xblastScriptEngine.h"
#include "string.h"
#include "stdlib.h"
#include <stddef.h>

typedef struct _singleChangeEntry{
    void *origPtr;
    void *modifiedOtr;
    unsigned char itemVariableSize;
    short arraySize;    //0 for no array
    bool isEEPROM;
    bool isBootScript;
    bool isString;
    char *displayString;
    struct _singleChangeEntry *nextChange;
}_singleChangeEntry;

typedef struct {
    unsigned short nbChanges;
    _singleChangeEntry *firstChangeEntry;
}_completeChangeList;

static _completeChangeList changeList;

unsigned char LPCMod_CountNumberOfChangesInSettings(void)
{
    int numberOfChanges = 0;

    unsigned char i, j;
    _settingsPtrStruct originalSettingsPtrStruct;
    setCFGFileTransferPtr(&LPCmodSettingsOrigFromFlash, &originalSettingsPtrStruct);

    for(i = 0; i < NBTXTPARAMS; i++){
        if(i < IPTEXTPARAMGROUP){
            if(*originalSettingsPtrStruct.settingsPtrArray[i] != *settingsPtrStruct.settingsPtrArray[i])
                numberOfChanges += 1;
        }
        else if(i < TEXTPARAMGROUP){
            for(j = 0; j < 4; j++){
                if(originalSettingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][j] != settingsPtrStruct.IPsettingsPtrArray[i - IPTEXTPARAMGROUP][j]){
                    numberOfChanges += 1;
                    break;
                }
            }
        }
        else if(i < SPECIALPARAMGROUP){
            if(strcmp(originalSettingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP], settingsPtrStruct.textSettingsPtrArray[i - TEXTPARAMGROUP]))
                numberOfChanges += 1;
        }
        else{
            if(*originalSettingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP] != *settingsPtrStruct.specialCasePtrArray[i - SPECIALPARAMGROUP])
                numberOfChanges += 1;
        }
    }

    numberOfChanges += generateStringsForEEPROMChanges(false); //do not generate strings

    if(LPCMod_checkForBootScriptChanges()){
        numberOfChanges += 1;
    }

    return numberOfChanges;
}


bool LPCMod_checkForBootScriptChanges(void){
    unsigned int oldBufferSize, modifiedBufferSize;
    if(scriptSavingPtr == NULL){
        return false;   //No new script to save. Therefore no changes.
    }

    if(bootScriptBuffer == NULL){
        return true;    //If there's a new script to save but nothing is already saved in flash. Change detected.
    }

    //If there's both a new script to save and a script already saved in flash. Check if they are different.
    modifiedBufferSize = LPCmodSettings.firstScript.nextEntryPosition - sizeof(_LPCmodSettings) - 1;
    oldBufferSize = LPCmodSettingsOrigFromFlash.firstScript.nextEntryPosition - sizeof(_LPCmodSettings) - 1;

    if(modifiedBufferSize != oldBufferSize){    //Scripts size differ.
        return true;
    }

    //If above condition is false, it means both script have the same size
    if(memcmp(scriptSavingPtr, bootScriptBuffer, oldBufferSize)){
        return true;
    }

    //Both scripts are identical.
    return false;
}

void cleanChangeListStruct(void){
    unsigned short i;
    _singleChangeEntry *tempEntryPtr;

    for(i = 0; i < changeList.nbChanges; i++){
        if(changeList.firstChangeEntry == NULL)
            break;

        if(changeList.firstChangeEntry->nextChange != NULL)
            tempEntryPtr = changeList.firstChangeEntry->nextChange;
        else
            tempEntryPtr = NULL;

        if(changeList.firstChangeEntry->displayString != NULL)
            free(changeList.firstChangeEntry->displayString);
        free(changeList.firstChangeEntry);
        changeList.firstChangeEntry = tempEntryPtr;
    }

    changeList.nbChanges = 0;
}

#endif /* XBLASTSETTINGSCHANGETRACKER_C_ */
