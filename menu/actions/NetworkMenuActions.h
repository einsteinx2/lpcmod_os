#ifndef _NETWORKMENUACTIONS_H_
#define _NETWORKMENUACTIONS_H_
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "boot.h"

#define IP_KEYPAD   1

void toggleUseDHCP(void * itemStr);

void editStaticIPfield(void * ip);

u8 myAtoi(char *str);

#endif
