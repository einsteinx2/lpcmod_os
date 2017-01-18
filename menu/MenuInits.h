/*
 * MenuInits.h
 *
 *  Created on: Sep 14, 2016
 *      Author: bennyboy
 */

#ifndef MENU_MENUINITS_H_
#define MENU_MENUINITS_H_

#include "MenuActions.h"
#include "stdlib.h"

void IconMenuInit(void);

// Generated only once. Static entries.
TEXTMENU* BankSelectMenuInit(void);
TEXTMENU* BFMBootMenuInit(void);
TEXTMENU* CDMenuInit(void);
TEXTMENU* DeveloperMenuInit(void);
TEXTMENU* EEPROMFileRestoreMenuInit(void);
TEXTMENU* eepromEditMenuDynamic(void);
TEXTMENU* HDDMenuInit(void);
TEXTMENU* InfoMenuInit(void);
TEXTMENU* LCDMenuInit(void);
TEXTMENU* LEDMenuInit(void);
TEXTMENU* ModchipMenuInit(void);
TEXTMENU* NetworkMenuInit(void);
TEXTMENU* ResetMenuInit(void);
TEXTMENU* SystemMenuInit(void);
TEXTMENU* TextMenuInit(void);
TEXTMENU* ToolsMenuInit(void);
TEXTMENU* VideoMenuInit(void);
TEXTMENU* XBlastScriptMenuInit(void);

// Generated on the fly. Must free any alloc on exit.
void UncommittedChangesMenuDynamic(void* unused);
void TSOPBankSelectMenuDynamic(void* bank);
void BankSelectDynamic(void* bank);
void HDDFlashMenuDynamic(void* unused);
void LargeHDDMenuDynamic(void* drive);

#endif /* MENU_MENUINITS_H_ */
