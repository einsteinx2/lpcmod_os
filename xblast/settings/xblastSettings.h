/*
 * xblastSettings.h
 *
 *  Created on: Aug 11, 2016
 *      Author: cromwelldev
 */

#ifndef XBLASTSETTINGS_H_
#define XBLASTSETTINGS_H_

#include "xblastSettingsDefs.h"

void populateSettingsStructWithDefault(_LPCmodSettings *LPCmodSettings);

void LPCMod_LCDBankString(char * string, unsigned char bankID);

const char* const getSpecialSettingTextString(unsigned char SpecialSettingindex, unsigned char value);
const char* const getSpecialSettingDisplayString(unsigned char SpecialSettingindex, unsigned char value);

bool emergencyRecoverSettings(void);

#endif /* XBLASTSETTINGS_H_ */
