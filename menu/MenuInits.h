/*
 * MenuInits.h
 *
 *  Created on: Sep 14, 2016
 *      Author: bennyboy
 */

#ifndef MENU_MENUINITS_H_
#define MENU_MENUINITS_H_

#include "TextMenu.h"

void IconMenuInit(void);

TEXTMENU *BankSelectMenuInit(void * bank);
TEXTMENU *TSOPBankSelectMenuInit(void * bank);
TEXTMENU* BankSelectInit(void *);
TEXTMENU* BFMBootMenuInit(void);
TEXTMENU *CDMenuInit(void);
TEXTMENU *DeveloperMenuInit(void);
TEXTMENU* EEPROMFileRestoreMenuInit(void);
TEXTMENU *eepromEditMenuInit(void);
TEXTMENU* HDDFlashMenuInit(void);
TEXTMENU *HDDMenuInit(void);
TEXTMENU *LargeHDDMenuInit(void * drive);
TEXTMENU *InfoMenuInit(void);
TEXTMENU *LCDMenuInit(void);
TEXTMENU* LEDMenuInit(void);
TEXTMENU *ModchipMenuInit(void);
TEXTMENU * NetworkMenuInit(void);
TEXTMENU* ResetMenuInit(void);
TEXTMENU *SystemMenuInit(void);
TEXTMENU *TextMenuInit(void);
TEXTMENU *ToolsMenuInit(void);
TEXTMENU *VideoMenuInit(void);
TEXTMENU* XBlastScriptMenuInit(void);

#endif /* MENU_MENUINITS_H_ */
