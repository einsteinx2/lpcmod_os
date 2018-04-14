/*
 * UncommittedChangesMenuActions.c
 *
 *  Created on: Jan 9, 2017
 *      Author: cromwelldev
 */

#include "UncommittedChangesMenuActions.h"
#include "MenuActions.h"
#include "misc/ConfirmDialog.h"
#include "xblast/settings/xblastSettingsChangeTracker.h"
#include "xblast/settings/xblastSettingsDefs.h"
#include "BootEEPROM.h"
#include "lib/LPCMod/xblastDebug.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include <stdbool.h>

typedef struct
{
    TEXTMENU** menu;
    OSSettingsChangeEntry_t* change;
}OSChangeRevert_t;

typedef struct
{
    TEXTMENU** menu;
    EEPROMChangeEntry_t* change;
}EEPROMChangeRevert_t;

TEXTMENU* generateMenuEntries(void)
{
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;

    cleanOSSettingsChangeListStruct(&osSettingsChangeList);
    int uncommittedChanges = LPCMod_CountNumberOfChangesInSettings(true, &osSettingsChangeList);
    cleanEEPROMSettingsChangeListStruct(&eepromChangeList);
    unsigned char eepromChanges = generateEEPROMChangeList(true, &eepromChangeList); // generate strings
    unsigned char bootScriptChange = LPCMod_checkForBootScriptChanges() ? 1 : 0;
    unsigned char backupEEPROMChange = LPCMod_checkForBackupEEPROMChange() ? 1 : 0;
    int totalChangesCount = uncommittedChanges + eepromChanges + bootScriptChange + backupEEPROMChange;
    unsigned int i;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    sprintf(menuPtr->szCaption, "Uncommitted Change%s:", totalChangesCount > 1 ? "s" : "");
    menuPtr->smallChars = true;
    menuPtr->visibleCount = 20;
    menuPtr->hideUncommittedChangesLabel = true;


    if(totalChangesCount == 0)
    {
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "No uncommitted change.");
        TextMenuAddItem(menuPtr, itemPtr);
    }
    else
    {
        OSSettingsChangeEntry_t* currentOSChangeEntry = osSettingsChangeList.firstChangeEntry;
        EEPROMChangeEntry_t* currentEEPROMChangeEntry =  eepromChangeList.firstChangeEntry;
        for(i = 0; i < totalChangesCount; i++)
        {
            if(i < uncommittedChanges)
            {
              if(currentOSChangeEntry != NULL)
              {
                  debugSPIPrint(DEBUG_GENERAL_UI, "%s%s\n", currentOSChangeEntry->label, currentOSChangeEntry->changeString);
                  itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                  sprintf(itemPtr->szCaption, "%s ", currentOSChangeEntry->label);
                  strcpy(itemPtr->szParameter, currentOSChangeEntry->changeString);
#ifdef DEV_FEATURES
                  itemPtr->functionPtr = revertOSChange;
                  OSChangeRevert_t* param = malloc(sizeof(OSChangeRevert_t));
                  param->menu = &menuPtr;
                  param->change = currentOSChangeEntry;
                  itemPtr->functionDataPtr = param;
                  itemPtr->dataPtrAlloc = true;
#endif
                  TextMenuAddItem(menuPtr, itemPtr);

                  currentOSChangeEntry = currentOSChangeEntry->nextChange;
              }
            }
            else if(i < (uncommittedChanges + bootScriptChange))
            {
                debugSPIPrint(DEBUG_GENERAL_UI, "Boot script in flash change\n");
                itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                sprintf(itemPtr->szCaption, "Boot script in flash modified");
#ifdef DEV_FEATURES
                itemPtr->functionPtr = revertBootScriptChange;
                itemPtr->functionDataPtr = &menuPtr;
#endif
                TextMenuAddItem(menuPtr, itemPtr);
            }
            else if(i < (uncommittedChanges + bootScriptChange + backupEEPROMChange))
            {
                debugSPIPrint(DEBUG_GENERAL_UI, "Backup EEPROM modified\n");
                itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                sprintf(itemPtr->szCaption, "Backup EEPROM modified");
#ifdef DEV_FEATURES
                itemPtr->functionPtr = revertBackupEEPROMChange;
                itemPtr->functionDataPtr = &menuPtr;
#endif
                TextMenuAddItem(menuPtr, itemPtr);
            }
            else if(i < (uncommittedChanges + bootScriptChange + backupEEPROMChange + eepromChanges))
            {
                debugSPIPrint(DEBUG_GENERAL_UI, "New EEPROM change #%u\n", i);
                if(currentEEPROMChangeEntry != NULL)
                {
                    debugSPIPrint(DEBUG_GENERAL_UI, "%s%s\n", currentEEPROMChangeEntry->label, currentEEPROMChangeEntry->changeString);
                    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                    strcpy(itemPtr->szCaption, currentEEPROMChangeEntry->label);
                    strcpy(itemPtr->szParameter, currentEEPROMChangeEntry->changeString);
#ifdef DEV_FEATURES
                    itemPtr->functionPtr = revertEEPROMChange;
                    EEPROMChangeRevert_t* param = malloc(sizeof(EEPROMChangeRevert_t));
                    param->menu = &menuPtr;
                    param->change = currentEEPROMChangeEntry;
                    itemPtr->functionDataPtr = param;
                    itemPtr->dataPtrAlloc = true;
#endif
                    TextMenuAddItem(menuPtr, itemPtr);

                    currentEEPROMChangeEntry = currentEEPROMChangeEntry->nextChange;
                }
            }
        }
    }

    return menuPtr;
}

void revertOSChange(void* customSruct)
{
    OSChangeRevert_t* param = (OSChangeRevert_t *)customSruct;
    TEXTMENU** menuPtr = param->menu;
    OSSettingsChangeEntry_t* changeEntry = param->change;

    char confirmString[120];
    sprintf(confirmString, "Revert change :\n%s %s", changeEntry->label, changeEntry->changeString);

    if(ConfirmDialog(confirmString, true))
    {
        return;
    }

    memcpy(changeEntry->newSetting, changeEntry->origSettings, changeEntry->settingSize);
    freeTextMenuAllocMem(*menuPtr);
    *menuPtr = generateMenuEntries();
}

void revertBootScriptChange(void* menuPtr)
{
    TEXTMENU** menu = (TEXTMENU **)menuPtr;

    if(ConfirmDialog("Revert change :\nBoot script in flash modified", true))
    {
        return;
    }

    LPCmodSettings.flashScript.scriptSize = LPCmodSettingsOrigFromFlash.flashScript.scriptSize;
    memcpy(LPCmodSettings.flashScript.scriptData, LPCmodSettingsOrigFromFlash.flashScript.scriptData, LPCmodSettings.flashScript.scriptSize);

    freeTextMenuAllocMem(*menu);
    *menu = generateMenuEntries();
}

void revertBackupEEPROMChange(void* menuPtr)
{
    TEXTMENU** menu = (TEXTMENU **)menuPtr;

    if(ConfirmDialog("Revert change :\nBackup EEPROM modified", true))
    {
        return;
    }

    memcpy(&LPCmodSettings.bakeeprom, &LPCmodSettingsOrigFromFlash.bakeeprom, sizeof(EEPROMDATA));

    freeTextMenuAllocMem(*menu);
    *menu = generateMenuEntries();
}

void revertEEPROMChange(void* customStruct)
{
    EEPROMChangeRevert_t* param = (EEPROMChangeRevert_t *)customStruct;
    TEXTMENU** menuPtr = param->menu;
    EEPROMChangeEntry_t* changeEntry = param->change;

    char confirmString[120];
    sprintf(confirmString, "Revert change :\n%s %s", changeEntry->label, changeEntry->changeString);

    if(ConfirmDialog(confirmString, true))
    {
        return;
    }

    freeTextMenuAllocMem(*menuPtr);
    *menuPtr = generateMenuEntries();
}
