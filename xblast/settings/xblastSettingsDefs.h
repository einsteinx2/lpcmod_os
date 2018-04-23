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
#define _SettingsMaxTextFieldsLength _SettingsMaxTextFieldsLength_V1
#define CurrentSettingsVersionNumber 1

// Globals
extern _LPCmodSettings LPCmodSettings;
extern _LPCmodSettings LPCmodSettingsOrigFromFlash;
extern const unsigned char LPCmodSettingsTextFieldsMaxLength;


//Items below should always follow the current settings version
#define MINPARAMLENGTH 7
#define BoolParamGroup 12
#define NumParamGroup 7
#define IPParamGroup 5
#define TextParamGroup 8
#define SpecialParamGroup 4

typedef struct {
    unsigned char *boolSettingsPtrArray[BoolParamGroup];
    unsigned char *numSettingsPtrArray[NumParamGroup];
    unsigned char *IPsettingsPtrArray[IPParamGroup];
    char *textSettingsPtrArray[TextParamGroup];
    unsigned char *specialCasePtrArray[SpecialParamGroup];
}_settingsPtrStruct;

enum SpecialSettingsPtrArrayIndexName
{
    SpecialSettingsPtrArrayIndexName_ActiveBank = 0u,
    SpecialSettingsPtrArrayIndexName_AltBank,
    SpecialSettingsPtrArrayIndexName_LEDColor,
    SpecialSettingsPtrArrayIndexName_LCDType
};

_settingsPtrStruct settingsPtrStruct;

typedef struct {
    const char *boolSettingsStringArray[BoolParamGroup];
    const char *numSettingsStringArray[NumParamGroup];
    const char *IPsettingsStringArray[IPParamGroup];
    const char *textSettingsStringArray[TextParamGroup];
    const char *specialSettingsStringArray[SpecialParamGroup];
}_xblastCfgStringsStruct;
const _xblastCfgStringsStruct xblastCfgStringsStruct;

#endif /* XBLASTSETTINGSDEFS_H_ */
