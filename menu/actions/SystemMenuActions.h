#ifndef _SYSTEMMENUACTIONS_H_
#define _SYSTEMMENUACTIONS_H_
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

void incrementFanSpeed(void * itemStr);
void decrementFanSpeed(void * itemStr);

void incrementGameRegion(void * itemStr);
void decrementGameRegion(void * itemStr);

void incrementDVDRegion(void * itemStr);
void decrementDVDRegion(void * itemStr);

void toggleROMBus(void * itemStr);

#endif
