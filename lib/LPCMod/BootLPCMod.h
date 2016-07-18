/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _BootLPCMod_H_
#define _BootLPCMod_H_

#include "VideoInitialization.h"

#define HDD4780_DEFAULT_NBLINES    4
#define HDD4780_DEFAULT_LINELGTH    20
#define DEFAULT_FANSPEED    20

#define NBTXTPARAMS 35
#define MINPARAMLENGTH 7
#define IPTEXTPARAMGROUP 18
#define TEXTPARAMGROUP (IPTEXTPARAMGROUP + 5)
#define SPECIALPARAMGROUP (TEXTPARAMGROUP + 8)

#define NBBOOLEANPARAMS 11
#define NBNUMERICVALUEPARAMS (IPTEXTPARAMGROUP - NBBOOLEANPARAMS)

extern char *xblastcfgstrings[NBTXTPARAMS];

typedef struct {
    unsigned char *settingsPtrArray[IPTEXTPARAMGROUP];
    unsigned char *IPsettingsPtrArray[TEXTPARAMGROUP-IPTEXTPARAMGROUP];
    char *textSettingsPtrArray[SPECIALPARAMGROUP - TEXTPARAMGROUP];
    unsigned char *specialCasePtrArray[NBTXTPARAMS - SPECIALPARAMGROUP];
}_settingsPtrStruct;

_settingsPtrStruct settingsPtrStruct;

typedef struct _singleChangeEntry{
    void *origPtr;
    void *modifiedOtr;
    u8 itemVariableSize;
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

_completeChangeList changeList;

void initialLPCModOSBoot(_LPCmodSettings *LPCmodSettings);

u16 LPCMod_HW_rev(void);

void LPCMod_ReadIO(struct _GenPurposeIOs *GPIOstruct);
void LPCMod_WriteIO(u8 port, u8 value);
void LPCMod_FastWriteIO(u8 port, u8 value);

void LPCMod_WriteGenPurposeIOs(void);

void LPCMod_LCDBankString(char * string, u8 bankID);

int LPCMod_ReadCFGFromHDD(_LPCmodSettings *LPCmodSettingsPtr, _settingsPtrStruct *settingsStruct);
int LPCMod_SaveCFGToHDD(void);

u8 LPCMod_CountNumberOfChangesInSettings(void);
bool LPCMod_checkForBootScriptChanges(void);
void cleanChangeListStruct(void);

#endif // _BootLPCMod_H_
