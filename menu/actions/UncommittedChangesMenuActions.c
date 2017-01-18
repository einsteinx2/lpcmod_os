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
    int totalChangesCount = uncommittedChanges + eepromChanges + bootScriptChange;
    unsigned int i;

    menuPtr = calloc(1, sizeof(TEXTMENU));
    sprintf(menuPtr->szCaption, "Uncommitted Change%s:", totalChangesCount > 1 ? "s" : "");
    menuPtr->smallChars = true;
    menuPtr->visibleCount = 20;
    menuPtr->hideUncommittedChangesLabel = true;


    if(totalChangesCount == 0)
    {
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption, "%s", "No uncommitted change.");
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
                  debugSPIPrint("%s%s\n", currentOSChangeEntry->label, currentOSChangeEntry->changeString);
                  itemPtr = malloc(sizeof(TEXTMENUITEM));
                  memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
                  sprintf(itemPtr->szCaption, "%s ", currentOSChangeEntry->label);
                  sprintf(itemPtr->szParameter, "%s", currentOSChangeEntry->changeString);
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
                debugSPIPrint("Boot script in flash change\n");
                itemPtr = malloc(sizeof(TEXTMENUITEM));
                memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
                sprintf(itemPtr->szCaption, "Boot script in flash modified");
#ifdef DEV_FEATURES
                itemPtr->functionPtr = revertBootScriptChange;
                itemPtr->functionDataPtr = &menuPtr;
#endif
                TextMenuAddItem(menuPtr, itemPtr);
            }
            else if(i < (uncommittedChanges + bootScriptChange + eepromChanges))
            {
                debugSPIPrint("New EEPROM change #%u\n", i);
                if(currentEEPROMChangeEntry != NULL)
                {
                    debugSPIPrint("%s%s\n", currentEEPROMChangeEntry->label, currentEEPROMChangeEntry->changeString);
                    itemPtr = malloc(sizeof(TEXTMENUITEM));
                    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
                    sprintf(itemPtr->szCaption,"%s", currentEEPROMChangeEntry->label);
                    sprintf(itemPtr->szParameter,"%s", currentEEPROMChangeEntry->changeString);
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
    memcpy(LPCmodSettingsOrigFromFlash.flashScript.scriptData, LPCmodSettingsOrigFromFlash.flashScript.scriptData, LPCmodSettings.flashScript.scriptSize);

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
