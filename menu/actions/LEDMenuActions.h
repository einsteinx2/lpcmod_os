#ifndef _LEDMENUACTIONS_H_
#define _LEDMENUACTIONS_H_
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define LED_OFF	0x0;
#define LED_GREEN	0x1;
#define LED_RED		0x2;
#define LED_ORANGE 0x3;
#define LED_CYCLE	0x4;
#define LED_FIRSTBOOT	0x5;

void LEDGreen(void *);
void LEDRed(void *);
void LEDOrange(void *);
void LEDCycle(void *);
void LEDOff(void *);
void LEDFirstBoot(void *);

#endif
