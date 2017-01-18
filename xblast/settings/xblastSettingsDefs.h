/*
 * xblastSettingsDefs.h
 *
 *  Created on: Aug 11, 2016
 *      Author: cromwelldev
 */

#ifndef XBLASTSETTINGSDEFS_H_
#define XBLASTSETTINGSDEFS_H_

#include "XBlastSettingsDefs_V1.h"

//Current settings version
//Change when newer version is made
//Migrate settings to current version(if necessary) on fetch from flash to avoid issues.
typedef _LPCmodSettings_V1 _LPCmodSettings;
#define CurrentSettingsVersionNumber 1

// Globals
_LPCmodSettings LPCmodSettings;
_LPCmodSettings LPCmodSettingsOrigFromFlash;


//Items below should always follow the current settings version
#define NBTXTPARAMS 36
#define MINPARAMLENGTH 7
#define IPTEXTPARAMGROUP 19 //Starting offset
#define TEXTPARAMGROUP (IPTEXTPARAMGROUP + 5)
#define SPECIALPARAMGROUP (TEXTPARAMGROUP + 8)

#define NBBOOLEANPARAMS 12
#define NBNUMERICVALUEPARAMS (IPTEXTPARAMGROUP - NBBOOLEANPARAMS)

typedef struct {
    unsigned char *settingsPtrArray[IPTEXTPARAMGROUP];
    unsigned char *IPsettingsPtrArray[TEXTPARAMGROUP-IPTEXTPARAMGROUP];
    char *textSettingsPtrArray[SPECIALPARAMGROUP - TEXTPARAMGROUP];
    unsigned char *specialCasePtrArray[NBTXTPARAMS - SPECIALPARAMGROUP];
}_settingsPtrStruct;

enum SpecialSettingsPtrArrayIndexName
{
    SpecialSettingsPtrArrayIndexName_ActiveBank = 0u,
    SpecialSettingsPtrArrayIndexName_AltBank,
    SpecialSettingsPtrArrayIndexName_LEDColor,
    SpecialSettingsPtrArrayIndexName_LCDType
};

_settingsPtrStruct settingsPtrStruct;

const char *xblastcfgstrings[NBTXTPARAMS];

#endif /* XBLASTSETTINGSDEFS_H_ */
