/*
 * xblastSettingsImportExport.h
 *
 *  Created on: Aug 11, 2016
 *      Author: cromwelldev
 */

#ifndef XBLASTSETTINGSIMPORTEXPORT_H_
#define XBLASTSETTINGSIMPORTEXPORT_H_

#include "xblastSettingsDefs.h"

int LPCMod_ReadCFGFromHDD(_LPCmodSettings *LPCmodSettingsPtr, _settingsPtrStruct *settingsStruct);
int LPCMod_SaveCFGToHDD(_settingsPtrStruct *settingsStruct);
void setCFGFileTransferPtr(_LPCmodSettings * tempLPCmodSettings, _settingsPtrStruct *settingsStruct);

void importNewSettingsFromCFGLoad(_LPCmodSettings* newSettings);

#endif /* XBLASTSETTINGSIMPORTEXPORT_H_ */
