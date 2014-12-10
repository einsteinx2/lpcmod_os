#ifndef _EEPROMEDITMENUACTIONS_H_
#define _EEPROMEDITMENUACTIONS_H_
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "boot.h"

void displayEditEEPROMBuffer(void *ignored);

void LastResortRecovery(void *ignored);

void bruteForceFixDisplayresult(void *ignored);
bool bruteForceFixEEprom(void);

void confirmSaveToEEPROMChip(void *ignored);

#endif
