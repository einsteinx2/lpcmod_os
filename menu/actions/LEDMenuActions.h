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

void LEDGood(void *);
void LEDError(void *);
void LEDBusy(void *);
void LEDImportant(void *);
void LEDInput(void *);
void LEDDownloading(void *);
void LEDUber(void *);
void LEDHigh(void *);
void LEDMid(void *);
void LEDLow(void *);
void LEDHeader(char *name, char *pattern);
void LEDFooter(void);

#endif
