#ifndef _LCDMENUACTIONS_H_
#define _LCDMENUACTIONS_H_
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

void LCDToggleEN5V(void * itemStr);
void LCDChangeLCDType(void * itemStr);
void LCDIncrementBacklight(void * itemStr);
void LCDDecrementBacklight(void * itemStr);
void LCDIncrementContrast(void * itemStr);
void LCDDecrementContrast(void * itemStr);
void LCDToggleDisplayBootMsg(void * itemStr);
void LCDToggledisplayBIOSNameBoot(void * itemStr);
void LCDToggledisplayCustomTextBoot(void * itemStr);
void editCustomString0(void *whatever);
void editCustomString1(void *whatever);
void editCustomString2(void *whatever);
void editCustomString3(void *whatever);

void LCDDecreaseNbLines(void * itemStr);
void LCDIncreaseNbLines(void * itemStr);
void LCDDecreaseLineLength(void * itemStr);
void LCDIncreaseLineLength(void * itemStr);

#endif
