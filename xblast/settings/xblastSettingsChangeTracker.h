/*
 * xblastSettingsChangeTracker.h
 *
 *  Created on: Aug 11, 2016
 *      Author: cromwelldev
 */

#ifndef XBLASTSETTINGSCHANGETRACKER_H_
#define XBLASTSETTINGSCHANGETRACKER_H_

#include <stdbool.h>

typedef struct OSSettingsChangeEntry
{
    const char* label;
    char* changeString;
    void* newSetting;
    void* origSettings;
    unsigned char settingSize;
    struct OSSettingsChangeEntry* nextChange;
}OSSettingsChangeEntry_t;

typedef struct
{
    unsigned char changeCount;
    OSSettingsChangeEntry_t* firstChangeEntry;
}OSSettingsChangeList;

OSSettingsChangeList osSettingsChangeList;

void settingsTrackerInit(void);

unsigned char LPCMod_CountNumberOfChangesInSettings(bool generateChangeStruct, OSSettingsChangeList* output);
bool LPCMod_checkForBootScriptChanges(void);
bool LPCMod_checkForBackupEEPROMChange(void);
void cleanOSSettingsChangeListStruct(OSSettingsChangeList* input);

#endif /* XBLASTSETTINGSCHANGETRACKER_H_ */
