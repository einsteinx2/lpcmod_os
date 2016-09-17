/*
 * xblastSettingsChangeTracker.h
 *
 *  Created on: Aug 11, 2016
 *      Author: cromwelldev
 */

#ifndef XBLASTSETTINGSCHANGETRACKER_H_
#define XBLASTSETTINGSCHANGETRACKER_H_

#include <stdbool.h>

unsigned char LPCMod_CountNumberOfChangesInSettings(void);
bool LPCMod_checkForBootScriptChanges(void);
void cleanChangeListStruct(void);

#endif /* XBLASTSETTINGSCHANGETRACKER_H_ */
