#ifndef _TOOLSMENUACTIONS_H_
#define _TOOLSMENUACTIONS_H_
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#define NBTXTPARAMS 31
#define IPTEXTPARAMGROUP 14
#define TEXTPARAMGROUP IPTEXTPARAMGROUP + 5
#define SPECIALPARAMGROUP TEXTPARAMGROUP + 8
extern char *xblastcfgstrings[NBTXTPARAMS];
extern unsigned char *settingsPtrArray[];
unsigned char *IPsettingsPtrArray[TEXTPARAMGROUP-IPTEXTPARAMGROUP];
extern char *textSettingsPtrArray[];
extern unsigned char *specialCasePtrArray[];

void saveEEPromToFlash(void *whatever);
void restoreEEPromFromFlash(void *whatever);
void warningDisplayEepromEditMenu(void *ignored);
void wipeEEPromUserSettings(void *whatever);

void showMemTest(void *whatever);
void memtest(void);
int testBank(int bank);

void ToolHeader(char *title);
void ToolFooter(void);

//void TSOPRecoveryReboot(void *ignored);

void saveXBlastcfg(void * ignored);
void loadXBlastcfg(void * ignored);

void prevA19controlModBootValue(void * itemPtr);
void nextA19controlModBootValue(void * itemPtr);

#endif
