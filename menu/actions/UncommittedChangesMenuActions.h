/*
 * UncommittedChangesMenuActions.h
 *
 *  Created on: Jan 9, 2017
 *      Author: cromwelldev
 */

#ifndef UNCOMMITTEDCHANGESMENUACTIONS_H_
#define UNCOMMITTEDCHANGESMENUACTIONS_H_

#include "TextMenu.h"

TEXTMENU* generateMenuEntries(void);

void revertOSChange(void* customSruct);
void revertBootScriptChange(void* menuPtr);
void revertBackupEEPROMChange(void* menuPtr);
void revertEEPROMChange(void* customStruct);

#endif /* UNCOMMITTEDCHANGESMENUACTIONS_H_ */
