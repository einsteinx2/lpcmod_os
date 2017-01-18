#ifndef _MODCHIPMENUACTIONS_H_
#define _MODCHIPMENUACTIONS_H_
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "MenuInits.h"

#define BIOSNAMEMAXLENGTH       20

typedef struct
{
    char* powerButString;
    char* ejectButString;
    char* tsopControlString;
    TEXTMENUITEM* bank256ItemPtr;
    TEXTMENUITEM* tsopBank0ItemPtr;
    TEXTMENUITEM* tsopBank1ItemPtr;
    TEXTMENUITEM* tsopFullItemPtr;
    TEXTMENUITEM* resetAllItemPtr;
}BankSelectCommonParams;

void decrementActiveBank(void* itemStr);
void incrementActiveBank(void* itemStr);

void decrementAltBank(void* itemStr);
void incrementAltBank(void* itemStr);

void decrementbootTimeout(void* itemStr);
void incrementbootTimeout(void* itemStr);

void toggleQuickboot(void* itemStr);

void resetSettings(void* ignored);

void editBIOSName(void* bankID);

void toggleTSOPcontrol(void* customStruct);
void toggleTSOPhide(void* itemStr);

void reorderTSOPNameMenuEntries(BankSelectCommonParams* params);

#endif
