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

#define BIOSNAMEMAXLENGTH       20

void decrementActiveBank(void * itemStr);
void incrementActiveBank(void * itemStr);

void decrementAltBank(void * itemStr);
void incrementAltBank(void * itemStr);

void decrementbootTimeout(void * itemStr);
void incrementbootTimeout(void * itemStr);

void toggleQuickboot(void * itemStr);

void resetSettings(void *whatever);

void editBIOSName(void *bankID);

void toggleTSOPControl(void * itemPtr);
void toggleTSOPSplit(void * itemPtr);

#endif
