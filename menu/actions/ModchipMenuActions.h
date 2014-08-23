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

void decrementActiveBank(void * itemStr);
void incrementActiveBank(void * itemStr);

void decrementbootTimeout(void * itemStr);
void incrementbootTimeout(void * itemStr);

void toggleQuickboot(void * itemStr);

void resetSettings(void *whatever);

void toggleTSOPControl(void * itemStr);
void toggleTSOPSplit(void * itemStr);

#endif
