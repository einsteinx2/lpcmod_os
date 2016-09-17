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
void editStaticIP(void * itemStr);
void editStaticMask(void * itemStr);
void editStaticGateway(void * itemStr);
void editStaticDNS1(void * itemStr);
void editStaticDNS2(void * itemStr);

bool editIPfield(unsigned char * addr);

unsigned short myAtoi(char *str);
bool assertCorrectIPString(unsigned char *out, char *in);

#endif
